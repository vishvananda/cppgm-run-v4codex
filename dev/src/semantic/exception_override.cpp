#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::check_exception_override(EntityId function, EntityId base)
{
    // [except.spec]/5,8: an override may allow only exceptions matched by
    // the base specification. These are declaration facts, never body demand.
    // Preserve the cheap unrestricted/nonthrowing path for ordinary methods.
    if (function_nonthrowing(base)) {
        if (!function_nonthrowing(function)) throw std::runtime_error("looser virtual exception specification");
        return;
    }
    auto restricted = [&](EntityId e) {
        return function_exception_specification(e).dynamic_types ||
            (destructor_member(e) && !(entities[e].exception_spec & 3));
    };
    if (!restricted(base)) return;
    auto collect = [&](EntityId root, std::vector<TypeId>& result) {
        std::vector<EntityId> functions(1,root), classes;
        Index seen_functions, seen_classes;
        while (!functions.empty() || !classes.empty()) {
            if (!functions.empty()) {
                auto e = functions.back(); functions.pop_back();
                if (seen_functions.get(e)) continue;
                seen_functions.put(e,1);
                if (function_nonthrowing(e)) continue;
                const auto spec = function_exception_specification(e);
                if (spec.dynamic_types) {
                    result.insert(result.end(),allowed_exception_types.begin()+spec.allowed_begin,
                        allowed_exception_types.begin()+spec.allowed_begin+spec.allowed_count);
                    continue;
                }
                if (!destructor_member(e) || (entities[e].exception_spec & 3)) return false;
                classes.push_back(scopes[entities[e].owner].entity);
            } else {
                auto cls = classes.back(); classes.pop_back();
                if (seen_classes.get(cls)) continue;
                seen_classes.put(cls,1);
                auto add = [&](TypeId type) {
                    while (types[type].kind == TypeKind::Array) type = types[type].child;
                    if (!class_value(type)) return;
                    require_destructor_class(types[type].entity);
                    if (auto dtor = type_destructor(type)) functions.push_back(dtor);
                    else classes.push_back(types[type].entity);
                };
                for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
                    add(entities[bases[b].base].type);
                for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
                    auto e = declarations[d].entity;
                    if (nonstatic_field(e) && entities[e].owner == entities[cls].scope) add(entities[e].type);
                }
            }
        }
        std::sort(result.begin(),result.end());
        result.erase(std::unique(result.begin(),result.end()),result.end());
        return true;
    };
    std::vector<TypeId> allowed, actual;
    if (!collect(base,allowed)) return;
    if (!collect(function,actual)) throw std::runtime_error("unrestricted virtual override");
    Index public_paths;
    auto public_base = [&](TypeId from, TypeId to) {
        if (!class_value(from) || !class_value(to)) return false;
        auto derived = types[from].entity, target = types[to].entity;
        auto k = key(derived,target);
        if (auto old = public_paths.get(k)) return old == 2;
        auto path = base_path(from,target);
        bool found = false;
        if (base_adjustments[path].edge && !base_adjustments[path].ambiguous) {
            std::vector<EntityId> pending(1,derived); Index seen;
            for (unsigned i = 0; i < pending.size() && !found; ++i) {
                auto current = pending[i];
                if (current == target) { found = true; break; }
                if (seen.get(current)) continue;
                seen.put(current,1);
                for (auto b = access_base(current); b; b = bases[b].next)
                    if (bases[b].access == Access::Public) pending.push_back(bases[b].base);
            }
        }
        // Handler matching ignores lexical privileges and explicit-
        // instantiation access overrides, including private shared paths.
        public_paths.put(k,found ? 2 : 1); return found;
    };
    auto matches = [&](TypeId from, TypeId to) {
        if (public_base(from,to)) return true;
        if (fundamental(from,FT_NULLPTR_T))
            return pointer(to) || types[to].kind == TypeKind::MemberPointer;
        if (!pointer(from) || !pointer(to)) return false;
        auto a = types[from].child, b = types[to].child;
        unsigned added = 0;
        if (qualification(a,b,added)) return true;
        auto cv = [&](TypeId type) {
            while (types[type].kind == TypeKind::Array) type = types[type].child;
            return types[type].cv;
        };
        if (cv(a) & ~cv(b)) return false;
        return (fundamental(b,FT_VOID) && types[a].kind != TypeKind::Function) || public_base(a,b);
    };
    for (auto type : actual) {
        if (std::binary_search(allowed.begin(),allowed.end(),type)) continue;
        bool found = false;
        for (auto candidate : allowed) if (matches(type,candidate)) { found = true; break; }
        if (!found) throw std::runtime_error("looser virtual allowed exception types");
    }
}
} }
