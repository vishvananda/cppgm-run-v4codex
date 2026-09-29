#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
using syntax::Kind;
// Function-local forward facts. A location is a local declaration followed by
// direct non-reference field edges, never a field declaration alone. Each fact
// expires at an effect/control boundary; no callee or alias analysis is implied.
struct Analyzer::MemberPointerFlow {
    Analyzer& sem;
    Index locations, offsets, definitions;
    struct Location { EntityId root; std::uint64_t offset; };
    std::vector<Location> storage = std::vector<Location>(1);
    std::uint32_t next_offset = 1, boundary = 1;
    unsigned remaining = 4096, depth = 0;
    bool cleanup = false;
    struct Value { std::uint32_t location = 0; bool zero = false; };
    explicit MemberPointerFlow(Analyzer& s) : sem(s) {}
    void barrier() { ++boundary; }
    std::uint32_t location(EntityId root, std::uint64_t offset) {
        auto offset_id = offsets.get(offset);
        if (!offset_id) { offset_id = next_offset++; offsets.put(offset,offset_id); }
        auto key = sem.key(root,offset_id);
        if (auto known = locations.get(key)) return known;
        auto result = storage.size(); storage.push_back({root,offset});
        locations.put(key,result); return result;
    }
    Value visit(NodeId n) {
        if (!n) return Value();
        if (!remaining || depth == 64) { barrier(); cleanup = true; return Value(); }
        --remaining; ++depth; ++sem.member_pointer_flow_work;
        auto result = node(n);
        auto x = sem.expressions[n];
        auto incoming = sem.conversions[x.incoming];
        if (incoming.function || (incoming.kind != Conversion::Kind::Standard &&
            incoming.kind != Conversion::Kind::Explicit && incoming.kind != Conversion::Kind::Contextual)) {
            barrier(); cleanup = true; result = Value();
        }
        if (incoming.derived && sem.base_adjustments[incoming.adjustment].total) result.zero = false;
        if (sem.types[x.type].cv & 2) result.zero = false;
        auto temporary = sem.object_fact(n).temporary;
        cleanup |= temporary && sem.destructor_needed(sem.object_destructor(temporary));
        --depth; return result;
    }
    void full_expression(NodeId n) {
        visit(n);
        if (cleanup) barrier();
        cleanup = false;
    }
    Value node(NodeId n) {
        auto node = sem.ast[n]; auto x = sem.expressions[n];
        Value result;
        if (x.form == ExpressionForm::Construction || x.form == ExpressionForm::OperatorCall ||
            x.form == ExpressionForm::LiteralCall || x.form == ExpressionForm::Cast) {
            barrier(); cleanup = true; return result;
        }
        switch (node.kind) {
        case Kind::Compound:
            barrier();
            for (auto c = node.first; c && remaining; c = sem.ast[c].next) {
                cleanup = false; visit(c);
                if (cleanup) barrier();
                cleanup = false;
            }
            barrier(); break; // Includes implicit destruction on scope exit.
        case Kind::ExpressionStatement: case Kind::Return:
            full_expression(node.first); break;
        case Kind::Parenthesized:
            result = visit(node.first); break;
        case Kind::IdExpression: {
            auto e = sem.entities[x.entity]; auto scope = sem.scopes[e.owner].kind;
            if (e.kind == EntityKind::Variable && !e.is_static && !e.external_decl && !e.thread_local_storage &&
                (scope == ScopeKind::Block || scope == ScopeKind::Control) &&
                sem.types[e.type].kind != TypeKind::LRef && sem.types[e.type].kind != TypeKind::RRef &&
                (!sem.class_value(e.type) || sem.entities[sem.types[e.type].entity].key != KW_UNION))
                result.location = location(x.entity,0);
            result.zero = sem.member_pointer_zero_adjustment(x.entity);
            break;
        }
        case Kind::Member: {
            auto base = visit(node.first);
            if (sem.object_fact(n).arrow) { barrier(); cleanup = true; }
            auto type = sem.types[x.type];
            if (node.op == OP_DOT && base.location && sem.nonstatic_field(x.entity) &&
                !sem.injected_storage(x.entity) &&
                type.kind != TypeKind::LRef && type.kind != TypeKind::RRef &&
                (!sem.class_value(x.type) || sem.entities[type.entity].key != KW_UNION) &&
                sem.types[sem.entities[x.entity].type].kind != TypeKind::LRef &&
                sem.types[sem.entities[x.entity].type].kind != TypeKind::RRef) {
                auto parent = storage[base.location];
                auto use = sem.object_fact(n);
                auto adjustment = sem.base_adjustments[use.qualifier_adjustment].total + sem.base_adjustments[use.adjustment].total;
                result.location = location(parent.root,parent.offset+adjustment+sem.entities[x.entity].member_offset);
            }
            result.zero = result.location && definitions.get(result.location) == boundary && !(type.cv & 2);
            if (result.zero && type.kind == TypeKind::MemberPointer) {
                sem.member_pointer_read_facts.put(n,1); ++sem.member_pointer_flow_reads;
            }
            break;
        }
        case Kind::Assignment: {
            // Consume the same recorded RHS-before-destination order as lowering.
            auto right = visit(sem.ast[node.first].next);
            auto left = visit(node.first);
            if (node.op != OP_ASS || !left.location || sem.class_value(x.type)) { barrier(); break; }
            definitions.put(left.location,right.zero ? boundary : 0);
            result = left; result.zero = right.zero; break;
        }
        case Kind::Unary: case Kind::Literal: case Kind::KeywordLiteral: {
            // The existing bounded value proof checks incoming adjustments and
            // accepts only checked member addresses/nulls, never overloaded &.
            if (sem.types[x.type].kind == TypeKind::MemberPointer) {
                unsigned budget = 64;
                result.zero = sem.prove_member_pointer_value(n,x.type,budget);
            } else if (node.kind == Kind::Unary) { barrier(); cleanup = true; }
            break;
        }
        case Kind::Call: {
            auto use = sem.object_fact(n);
            // Ordinary indirect callees are evaluated after arguments. Member
            // pointers instead capture receiver/target before argument effects.
            if (use.node) visit(use.node);
            if (use.arrow) { barrier(); cleanup = true; }
            if (use.member_pointer) visit(use.member_pointer);
            for (unsigned j = 0; j < x.argument_count && remaining; ++j)
                visit(sem.call_argument(x,j));
            barrier(); cleanup = true; break;
        }
        case Kind::Binary:
            if (node.op == OP_LAND || node.op == OP_LOR) { barrier(); cleanup = true; break; }
            visit(node.first); visit(sem.ast[node.first].next); break;
        case Kind::Then: case Kind::Else:
            barrier(); visit(node.first); barrier(); break;
        case Kind::If: case Kind::While: case Kind::Do: case Kind::For:
            // No merge/fixed point: each region starts and ends unknown. A loop
            // body may establish facts only after a store in that iteration.
            for (auto c = node.first; c && remaining; c = sem.ast[c].next) {
                barrier(); visit(c); barrier();
            }
            break;
        default:
            // Declarations, labels, switches, conditional lvalues, aliases and
            // unmodeled effects use generic lowering. Never walk deferred syntax.
            barrier(); cleanup = true; break;
        }
        return result;
    }
};
bool Analyzer::member_pointer_read_zero(NodeId expression) const
{ return member_pointer_read_facts.get(expression) != 0; }
void Analyzer::prepare_member_pointer_flows()
{
    for (auto function : member_pointer_flow_functions) {
        if (entities[function].body_state != FactState::Success) continue;
        MemberPointerFlow flow(*this); flow.visit(entities[function].body);
    }
}
} }
