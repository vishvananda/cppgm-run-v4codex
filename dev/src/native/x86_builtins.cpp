#include "native/encoding.h"
#include "support/x86_builtins.h"
namespace native {
void Encoder::x86_builtin(const Instruction& i)
{
    using namespace cppgm;
    auto id = unsigned(i.args[1].bits);
    lowir_model::require(valid_x86_builtin(id),"invalid x86 operation");
    const auto& d = cppgm::x86_builtin(id);
    auto out = Operand::mem(XR_R10), a = Operand::mem(XR_R10,16), b = Operand::mem(XR_R10,32);
    auto short_vector = [](X86Type t) { return t == X86Type::Int2 || t == X86Type::Long1 || t == X86Type::Byte8 || t == X86Type::Short4; };
    auto vector_load = [&](Operand from) { form(short_vector(d.first) ? 0x0f7e : 0x0f10,32,14,from,0,0,short_vector(d.first) ? 0xf3 : 0); };
    auto vector_store = [&]() { form(0x0f11,32,14,out); };
    switch(d.form) {
    case X86Form::Binary:
        vector_load(d.reverse ? b : a);
        if (d.opcode == 0xf10 || d.opcode == 0xf12 || d.opcode == 0xf16) {
            form(0xf10,32,15,b);
            form(d.opcode,32,14,Operand::r(15),0,0,d.prefix);
        } else form(d.opcode,32,14,d.reverse ? a : b,d.opcode == 0xfc2 ? 1 : 0,d.predicate,d.prefix);
        vector_store(); return;
    case X86Form::Unary:
        vector_load(a); form(d.opcode,32,14,a,0,0,d.prefix); vector_store(); return;
    case X86Form::ToInt: {
        bool wide = d.result == X86Type::I64;
        form(d.opcode,wide ? 64 : 32,XR_R11,a,0,0,d.prefix);
        store(out,Operand::r(XR_R11),wide ? Type::I64 : Type::I32); return;
    }
    case X86Form::FromInt:
        vector_load(a); form(d.opcode,d.second == X86Type::I64 ? 64 : 32,14,b,0,0,d.prefix); vector_store(); return;
    case X86Form::Compare:
        vector_load(a); form(d.opcode,32,14,b,0,0,d.prefix);
        form(0xf90+d.predicate,8,0,Operand::r(XR_R11));
        load(Operand::r(XR_R11),Operand::r(XR_R11),Type::U8,false);
        store(out,Operand::r(XR_R11),Type::I32); return;
    case X86Form::Mask:
        vector_load(a); form(d.opcode,32,XR_R11,Operand::r(14),0,0,d.prefix);
        store(out,Operand::r(XR_R11),Type::I32); return;
    case X86Form::GetControl: form(d.opcode,32,d.predicate,out); return;
    case X86Form::SetControl: form(d.opcode,32,d.predicate,a); return;
    case X86Form::Fence:
        if (d.opcode == 0xfae) form(d.opcode,32,d.predicate,Operand::r(0));
        else { if (d.prefix) byte(d.prefix); if (d.opcode > 255) byte(d.opcode >> 8); byte(d.opcode); }
        return;
    case X86Form::PairToFloat:
        form(0xf7e,32,15,b,0,0,0xf3); // zero upper two integer lanes
        form(0xf5b,32,15,Operand::r(15));
        vector_load(a); form(0xf10,32,14,Operand::r(15),0,0,0xf2); vector_store(); return;
    case X86Form::FloatToPair:
        form(0xf7e,32,14,a,0,0,0xf3); // do not convert ignored upper input lanes
        form(d.opcode,32,14,Operand::r(14),0,0,d.prefix); vector_store(); return;
    case X86Form::LoadHalf:
        vector_load(a); load(Operand::r(XR_R11),b,Type::Ptr,false);
        form(d.opcode,32,14,Operand::mem(XR_R11),0,0,d.prefix); vector_store(); return;
    case X86Form::StoreHalf:
        form(0xf10,32,14,b); load(Operand::r(XR_R11),a,Type::Ptr,false);
        form(d.opcode,32,14,Operand::mem(XR_R11),0,0,d.prefix); return;
    case X86Form::StreamStore:
        load(Operand::r(XR_R11),a,Type::Ptr,false);
        if (d.second == X86Type::I32 || d.second == X86Type::I64 || d.second == X86Type::U64 || d.second == X86Type::Int2) {
            // MOVNTI has the same 64-bit memory semantics as the MMX store,
            // without borrowing x87/MMX state for an integer carrier.
            load(Operand::r(XR_R10),b,d.second == X86Type::I32 ? Type::I32 : Type::I64,false);
            form(0xfc3,d.second == X86Type::I32 ? 32 : 64,XR_R10,Operand::mem(XR_R11));
        } else { form(0xf10,32,14,b); form(d.opcode,32,14,Operand::mem(XR_R11),0,0,d.prefix); }
        return;
    case X86Form::Flush:
        load(Operand::r(XR_R11),a,Type::Ptr,false); form(d.opcode,32,d.predicate,Operand::mem(XR_R11)); return;
    case X86Form::LoadVector:
        load(Operand::r(XR_R11),a,Type::Ptr,false);
        form(d.opcode,32,14,Operand::mem(XR_R11),0,0,d.prefix); vector_store(); return;
    case X86Form::DynamicCompare: {
        vector_load(a);
        auto compare = [&](unsigned predicate) {
            // VEX.NDS.128.0F: xmm14 <- xmm14 cmp record input.
            byte(0xc4); byte(0x41);
            byte(8 | (d.prefix == 0x66 ? 1 : d.prefix == 0xf3 ? 2 : d.prefix == 0xf2 ? 3 : 0));
            byte(0xc2); modrm(14,b); byte(predicate);
        };
        if (i.args[2].bits < 32) { compare(unsigned(i.args[2].bits)); vector_store(); return; }
        load(Operand::r(XR_R11),Operand::mem(XR_R10,48),Type::U32,false);
        form(0x83,32,4,Operand::r(XR_R11),1,31);
        std::vector<std::size_t> exits;
        for (unsigned predicate=0; predicate<32; ++predicate) {
            std::size_t next=0;
            if (predicate != 31) { form(0x83,32,7,Operand::r(XR_R11),1,predicate); next=local_jump(XC_NE); }
            compare(predicate);
            if (predicate != 31) { exits.push_back(local_jump(-1)); local_target(next); }
        }
        for (auto exit : exits) local_target(exit);
        vector_store(); return;
    }
    case X86Form::MaskStore:
        vector_load(a);
        form(short_vector(d.first) ? 0xf7e : 0xf10,32,15,b,0,0,short_vector(d.first) ? 0xf3 : 0);
        mov(Operand::r(XR_R11),Operand::r(XR_RDI));
        load(Operand::r(XR_RDI),Operand::mem(XR_R10,48),Type::Ptr,false);
        form(0xff7,32,14,Operand::r(15),0,0,0x66);
        mov(Operand::r(XR_RDI),Operand::r(XR_R11)); return;
    default: throw lowir_model::ParseError("non-native x86 lane recipe");
    }
}
}
