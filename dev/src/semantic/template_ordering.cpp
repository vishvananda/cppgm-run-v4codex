#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
bool Analyzer::template_more_specialized(EntityId a, EntityId b, unsigned arguments, bool operator_call, bool conversion)
{
    if (!a || !b || !entities[a].specialization || !entities[b].specialization) return false;
    const bool call = arguments != ~0u;
    auto nonstatic = [&](EntityId e) { return entities[e].member_info && !entities[e].is_static; };
    bool object_parameter = operator_call && nonstatic(a) != nonstatic(b);
    auto original_a = entities[specialization_pattern(a)].type, original_b = entities[specialization_pattern(b)].type;
    auto owner = [&](EntityId e) { return operator_call && nonstatic(e) ? scopes[entities[e].owner].entity : 0; };
    // All inputs are stable identities. Probe before constructing nominated
    // shapes, so a completed comparison has O(1) average lookup cost even for
    // long parameter lists. Defaults and body demand cannot change the key.
    auto identity = intern_arguments({original_a,original_b,arguments,unsigned(operator_call),owner(a),owner(b),unsigned(conversion)});
    if (auto known = function_partial_ordering.get(identity)) { ++function_ordering_hits; return known == 2; }
    ++function_ordering_work;
    auto shape = [&](EntityId e) {
        auto pattern = specialization_pattern(e);
        auto t = types[entities[pattern].type];
        // Conversion ordering nominates only the return type. A one-element
        // sequence reuses the reference/cv transformations and completeness
        // checks without involving unused template head parameters.
        if (conversion) return types.function(types.fundamental(FT_VOID),{t.child},false);
        if (!call) return entities[pattern].type;
        std::vector<TypeId> parameters;
        bool member = operator_call && entities[pattern].member_info && !entities[pattern].is_static;
        if (member && object_parameter) {
            auto object = types.qualify(entities[scopes[entities[pattern].owner].entity].type,t.cv);
            parameters.push_back(types.compound(t.ref == RefQualifier::Rvalue ? TypeKind::RRef : TypeKind::LRef,object));
        }
        auto count = arguments-member;
        bool pack = t.count && types[types.parameters[t.offset+t.count-1]].kind == TypeKind::PackExpansion;
        // Keep a symbolic tail even when its actual expansion is empty:
        // partial ordering does not depend on a deduced pack's length.
        auto limit = pack ? t.count : std::min(t.count,count);
        for (unsigned j = 0; j < limit; ++j) parameters.push_back(types.parameters[t.offset+j]);
        return types.function(types.fundamental(FT_VOID),parameters,false);
    };
    auto x = shape(a), y = shape(b);
    auto unqualify = [&](TypeId type) {
        std::vector<Type> arrays;
        while (types[type].kind == TypeKind::Array || types[type].kind == TypeKind::DependentArray) {
            arrays.push_back(types[type]); type = types[type].child;
        }
        type = types.unqualified(type);
        for (auto i = arrays.rbegin(); i != arrays.rend(); ++i) type = types.compound(i->kind,type,i->bound);
        return type;
    };
    auto transformed = [&](TypeId signature) {
        if (!call && !conversion) return signature;
        auto t = types[signature]; std::vector<TypeId> params;
        for (unsigned j = 0; j < t.count; ++j) {
            auto p = types.parameters[t.offset+j];
            bool pack = types[p].kind == TypeKind::PackExpansion;
            if (pack) p = types[p].bound;
            p = unqualify(value_type(p));
            params.push_back(pack ? types.compound(TypeKind::PackExpansion,0,p) : p);
        }
        return types.function(t.child,params,false);
    };
    auto tx = transformed(x), ty = transformed(y);
    auto complete = [&](TypeId type, const Index& bindings) {
        // [temp.deduct.partial]/11: unused head parameters are irrelevant,
        // but parameters occurring only in a non-deduced context still count.
        // Traverse retained types/queries, never instantiate their classes.
        std::vector<ArgumentId> work(1,type); Index seen;
        for (unsigned i = 0; i < work.size(); ++i) {
            auto arg = work[i];
            if (!arg || seen.get(arg)) continue;
            seen.put(arg,1);
            EntityId parameter = 0;
            if (value_argument(arg)) {
                auto q = type_queries[argument_query(arg)];
                if (q.kind == QueryKind::TemplateValueParameter || q.kind == QueryKind::SizeofPack) {
                    if (q.entity && entities[q.entity].template_parameter) parameter = q.entity;
                    else if (q.entity) work.push_back(entities[q.entity].type);
                }
                if (q.type) work.push_back(q.type);
                auto args = argument_packs[q.arguments];
                for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
                for (unsigned j = 0; j < q.count; ++j) work.push_back(value_argument_id(query_edges[q.offset+j]));
            } else {
                auto t = types[arg];
                if (t.kind == TypeKind::Named) {
                    if (entities[t.entity].template_parameter) parameter = t.entity;
                    else if (entities[t.entity].specialization) {
                        auto primary = specialization_pattern(t.entity);
                        if (entities[primary].template_parameter) work.push_back(entities[primary].type);
                        auto args = specialization_arguments(t.entity);
                        for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
                    }
                }
                if (t.kind == TypeKind::ArgumentPack) {
                    auto args = pack_arguments(arg);
                    for (unsigned j = 0; j < args.count; ++j) work.push_back(argument_types[args.offset+j]);
                }
                if (t.kind == TypeKind::PackExpansion) work.push_back(t.bound);
                if (t.kind == TypeKind::Decltype) work.push_back(value_argument_id(t.entity));
                if (t.kind == TypeKind::DependentArray) work.push_back(value_argument_id(t.bound));
                if (t.child) work.push_back(t.child);
                for (unsigned j = 0; j < t.count; ++j) work.push_back(types.parameters[t.offset+j]);
            }
            if (parameter && !bindings.get(parameter)) return false;
        }
        return true;
    };
    Index xy, yx;
    bool accepts_a = deduce_type(ty,tx,yx,DeductionKind::PartialOrdering) && complete(ty,yx);
    bool accepts_b = deduce_type(tx,ty,xy,DeductionKind::PartialOrdering) && complete(tx,xy);
    bool result = accepts_a && !accepts_b;
    if (accepts_a && (call || conversion)) {
        auto left = types[x], right = types[y]; bool stricter = false, worse = false;
        auto cv = [&](TypeId p) {
            while (types[p].kind == TypeKind::Array || types[p].kind == TypeKind::DependentArray) p = types[p].child;
            return types[p].cv;
        };
        for (unsigned i = 0; i < std::min(left.count,right.count); ++i) {
            auto p = types[types.parameters[left.offset+i]], q = types[types.parameters[right.offset+i]];
            if (p.kind == TypeKind::PackExpansion) p = types[p.bound];
            if (q.kind == TypeKind::PackExpansion) q = types[q.bound];
            bool pr = p.kind == TypeKind::LRef || p.kind == TypeKind::RRef;
            bool qr = q.kind == TypeKind::LRef || q.kind == TypeKind::RRef;
            if (!pr || !qr) continue;
            if (p.kind == q.kind && cv(p.child) == cv(q.child)) continue;
            // Reference ties belong to each compared type, including when
            // another parameter deduces in only one direction. Otherwise a
            // crossed structural/cv advantage spuriously produces a winner.
            if (!accepts_b) {
                Index pq, qp;
                auto pt = unqualify(p.child), qt = unqualify(q.child);
                if (!deduce_type(pt,qt,pq,DeductionKind::PartialOrdering) ||
                    !deduce_type(qt,pt,qp,DeductionKind::PartialOrdering)) continue;
            }
            if (p.kind != q.kind) { stricter |= p.kind == TypeKind::LRef; worse |= q.kind == TypeKind::LRef; }
            else {
                auto pcv = cv(p.child), qcv = cv(q.child);
                worse |= (qcv & ~pcv) != 0; stricter |= (pcv & ~qcv) != 0;
            }
        }
        result = (result || stricter) && !worse;
    }
    function_partial_ordering.put(identity,result ? 2 : 1);
    return result;
}
} }
