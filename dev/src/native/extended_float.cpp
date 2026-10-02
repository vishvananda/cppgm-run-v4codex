#include "native/model.h"
#include "support/id_index.h"
#include <cstring>
#include <chrono>

namespace native {
namespace {
using namespace lowir_model;
bool extended(Type t) { return t==Type::F16 || t==Type::F128; }
const char* format(Type t) { return t==Type::F16?"hf":t==Type::F32?"sf":t==Type::F64?"df":t==Type::F80?"xf":"tf"; }
class Legalizer {
    Program& p;
    Pool<lowir_model::Instruction> instructions;
    Pool<lowir_model::Operand> operands;
    cppgm::IdIndex helpers;
    FunctionId owner;
    DebugLocation debug;
    using Arg = lowir_model::Operand;
    Type actual(Arg a, Type fallback) const {
        return a.kind==Arg::Temporary?p.values[a.ref-1].type:a.kind==Arg::Slot?p.slots[a.ref-1].type:fallback;
    }
    Arg append(lowir_model::Instruction i, std::initializer_list<Arg> args, ValueId destination=ValueId()) {
        i.debug=debug; i.operands.begin=operands.size(); i.operands.count=args.size();
        for (auto a:args) operands.push_back(a);
        if (i.result_type()!=Type()) {
            if (!destination) { lowir_model::Value v; v.owner=owner; p.values.push_back(v); destination=ValueId(p.values.size()); }
            i.destination=destination;
            auto& v=p.values[destination.index-1]; v.type=i.result_type(); v.defined=true; v.definition=instructions.size()+1;
            v.truth=i.opcode==Opcode::Compare;
        }
        instructions.push_back(i); return Arg::value(i.destination);
    }
    Arg helper(unsigned operation, const std::string& name, Type result, Type input, Arg a, Arg b=Arg(), bool binary=false, ValueId destination=ValueId()) {
        auto key=1+operation+(std::uint64_t(input.kind())<<8)+(std::uint64_t(result.kind())<<16);
        SymbolId symbol(helpers.get(key)); SignatureId signature;
        if (!symbol) {
            symbol=p.symbol(p.intern("@"+name));
            if (p.symbols[symbol.index-1].kind==Symbol::Unknown) {
                lowir_model::Function fn; fn.symbol=symbol; fn.declaration=true;
                p.functions.push_back(fn); auto id=p.functions.size();
                if (!p.function_order.empty()) p.function_order.push_back(FunctionId(id));
                lowir_model::Signature sig; sig.result=result; sig.boundary.unwind=CUM_NO;
                sig.parameters.begin=p.parameters.size(); sig.parameters.count=binary?2:1;
                for (unsigned n=0;n<sig.parameters.count;++n) { lowir_model::Parameter parameter; parameter.type=input; p.parameters.push_back(parameter); }
                p.signatures.push_back(sig); signature=SignatureId(p.signatures.size()); p.functions[id-1].signature=signature;
                auto& s=p.symbols[symbol.index-1]; s.kind=Symbol::FunctionSymbol; s.entity=id; s.metadata.linkage=LLM_C;
                s.metadata.object=p.intern(name);
            }
            helpers.put(key,symbol.index);
        }
        const auto& s=p.symbols[symbol.index-1]; require(s.kind==Symbol::FunctionSymbol,"floating helper is not a function");
        signature=p.functions[s.entity-1].signature;
        const auto& sig=p.signatures[signature.index-1];
        require(sig.result==result && sig.parameters.count==(binary?2u:1u),"incompatible floating helper signature");
        for (unsigned n=0;n<sig.parameters.count;++n) require(p.parameters[sig.parameters.begin+n].type==input,"incompatible floating helper parameter");
        lowir_model::Instruction call(Opcode::Call,result); call.signature=signature;
        return binary?append(call,{Arg::symbol(symbol),a,b},destination):append(call,{Arg::symbol(symbol),a},destination);
    }
    Arg conversion(Arg a, Type from, Type to, bool unsign=false, bool unsigned_result=false, ValueId destination=ValueId()) {
        if (from==to) { if (!destination) return a; return append(lowir_model::Instruction(Opcode::Copy,to),{a},destination); }
        if (from.integer() && to.integer() && from.width()==to.width()) return append(lowir_model::Instruction(Opcode::Copy,to),{a},destination);
        if (!extended(from) && !extended(to)) {
            lowir_model::Instruction i(Opcode::Convert,to); i.source_type=from;
            i.operation=from.floating()?to.floating()?(to.width()>from.width()?Operation::Fpext:Operation::Fptrunc):
                unsigned_result?Operation::Fptoui:Operation::Fptosi:to.floating()?unsign?Operation::Uitofp:Operation::Sitofp:
                to.width()<from.width()?Operation::Trunc:unsign?Operation::Zext:Operation::Sext;
            return append(i,{a},destination);
        }
        if (from.floating() && to.floating()) {
            auto name=std::string(to.width()>from.width()?"__extend":"__trunc")+format(from)+format(to)+"2";
            return helper(0,name,to,from,a,Arg(),false,destination);
        }
        if (from==Type::F16) { a=conversion(a,from,Type::F128); from=Type::F128; }
        if (from.integer()) {
            Type carrier=to==Type::F16 || from==Type::I128?Type::I128:Type::I64;
            a=conversion(a,from,carrier,unsign||unsigned_type(from));
            auto name=std::string(unsign||unsigned_type(from)?"__floatun":"__float")+(carrier==Type::I128?"ti":"di")+format(to);
            return helper(unsign||unsigned_type(from)?2:1,name,to,carrier,a,Arg(),false,destination);
        }
        Type carrier=to==Type::I128?Type::I128:Type::I64;
        bool u=unsigned_result||unsigned_type(to);
        auto name=std::string(u?"__fixuns":"__fix")+format(from)+(carrier==Type::I128?"ti":"di");
        auto r=helper(u?4:3,name,carrier,from,a,Arg(),false,to==carrier?destination:ValueId());
        return to==carrier?r:conversion(r,carrier,to,u,u,destination);
    }
    void instruction(lowir_model::Instruction i) {
        debug=i.debug;
        auto a=[&](unsigned n){return p.operands[i.operands.begin+n];};
        if (i.opcode==Opcode::Convert && (extended(i.type)||extended(i.source_type))) {
            conversion(a(0),i.source_type,i.type,i.operation==Operation::Uitofp,i.operation==Operation::Fptoui,i.destination); return;
        }
        if (i.opcode==Opcode::Branch && extended(actual(a(0),Type()))) {
            lowir_model::Instruction cmp(Opcode::Compare,actual(a(0),Type())); cmp.operation=Operation::Ne;
            // Source lowering already makes truth explicit; this also owns raw LowIR branches.
            Arg truth;
            if (cmp.type==Type::F16) {
                auto x=conversion(a(0),cmp.type,Type::F32); cmp.type=Type::F32;
                truth=append(cmp,{x,Arg::floating(0)});
            } else {
                auto r=helper(10,"__netf2",Type::I32,Type::F128,a(0),Arg::floating(0),true);
                cmp.type=Type::I32; truth=append(cmp,{r,Arg::integer(0)});
            }
            append(i,{truth,a(1),a(2)}); return;
        }
        if ((i.opcode==Opcode::Store || i.opcode==Opcode::Return) && i.operands.count) {
            auto from=actual(a(0),i.type);
            if (from!=i.type && (extended(from)||extended(i.type))) {
                auto v=conversion(a(0),from,i.type,unsigned_type(from),unsigned_type(i.type));
                if (i.opcode==Opcode::Store) append(i,{v,a(1)}); else append(i,{v}); return;
            }
        }
        if (i.type==Type::F16 && (i.opcode==Opcode::Binary || i.opcode==Opcode::Compare || (i.opcode==Opcode::Unary && i.operation==Operation::Not))) {
            auto x=conversion(a(0),Type::F16,Type::F32);
            auto y=i.operands.count==2?conversion(a(1),Type::F16,Type::F32):Arg();
            auto op=i; op.type=Type::F32; op.source_type=Type(); op.destination=ValueId();
            auto r=i.operands.count==2?append(op,{x,y},i.opcode==Opcode::Compare?i.destination:ValueId()):append(op,{x});
            if (i.opcode!=Opcode::Compare) conversion(r,Type::F32,Type::F16,false,false,i.destination); return;
        }
        if (i.type==Type::F128 && (i.opcode==Opcode::Binary || i.opcode==Opcode::Compare || (i.opcode==Opcode::Unary && i.operation==Operation::Not))) {
            bool comparison=i.opcode!=Opcode::Binary;
            auto op=i.opcode==Opcode::Unary?Operation::Eq:i.operation;
            const char* name=op==Operation::Add?"__addtf3":op==Operation::Sub?"__subtf3":op==Operation::Mul?"__multf3":op==Operation::Div?"__divtf3":
                op==Operation::Eq?"__eqtf2":op==Operation::Ne?"__netf2":op==Operation::Lt||op==Operation::Ult?"__lttf2":op==Operation::Le||op==Operation::Ule?"__letf2":op==Operation::Gt||op==Operation::Ugt?"__gttf2":"__getf2";
            auto r=helper(16+unsigned(op),name,comparison?Type::I32:Type::F128,Type::F128,a(0),i.operands.count==2?a(1):Arg::floating(0),true,comparison?ValueId():i.destination);
            if (comparison) {
                lowir_model::Instruction cmp(Opcode::Compare,Type::I32); cmp.operation=op;
                if (op>=Operation::Ult && op<=Operation::Uge) cmp.operation=Operation(unsigned(op)-unsigned(Operation::Ult)+unsigned(Operation::Lt));
                r=append(cmp,{r,Arg::integer(0)},i.opcode==Opcode::Compare?i.destination:ValueId());
                if (i.opcode==Opcode::Unary) conversion(r,Type::I64,Type::F128,false,false,i.destination);
            } return;
        }
        auto old=i.operands; i.operands.begin=operands.size();
        for (unsigned n=old.begin;n<old.end();++n) operands.push_back(p.operands[n]);
        if (i.destination) p.values[i.destination.index-1].definition=instructions.size()+1;
        instructions.push_back(i);
    }
public:
    explicit Legalizer(Program& program):p(program) {}
    void run() {
        instructions.reserve(p.instructions.size()); operands.reserve(p.operands.size());
        for (auto& block:p.blocks) {
            owner=block.owner; auto old=block.instructions; block.instructions.begin=instructions.size();
            for (unsigned n=old.begin;n<old.end();++n) instruction(p.instructions[n]);
            block.instructions.count=instructions.size()-block.instructions.begin;
        }
        p.instructions.swap(instructions); p.operands.swap(operands);
    }
};
}
void legalize_extended_floats(lowir_model::Program& p, Statistics& stats) {
    auto begin=std::chrono::steady_clock::now();
    // No work buffer or helper declarations for the ordinary formats. This is
    // target legalization, once before selection: each input emits at most six
    // operations, no fixed point, speculative optimization or semantic lookup.
    bool needed=false;
    for (const auto& i:p.instructions) if (extended(i.type)||extended(i.source_type)) { needed=true; break; }
    // A raw LowIR branch can consume a floating parameter or bit-initialized
    // slot without any other floating instruction. Consult their typed pools.
    if (!needed) for (const auto& v:p.values) if (extended(v.type)) { needed=true; break; }
    if (!needed) for (const auto& s:p.slots) if (extended(s.type)) { needed=true; break; }
    if (needed) {
        auto input=p.instructions.size(), functions=p.functions.size();
        Legalizer(p).run(); stats.extended_work+=input; stats.extended_added+=p.instructions.size()-input;
        stats.extended_helpers+=p.functions.size()-functions;
    }
    stats.preparation_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
}
}
