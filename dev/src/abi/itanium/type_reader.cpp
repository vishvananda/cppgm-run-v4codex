#include "abi/itanium/fact_reader.h"
#include <algorithm>
#include <stdexcept>

namespace abi_mangle {
Id FactReader::compact(const std::string& word) {
    // Single ':' delimits constructors; '::' belongs to a qualified name.
    std::size_t colon = word.find(':');
    if (colon == std::string::npos || (colon + 1 < word.size() && word[colon + 1] == ':')) {
        Binding b = lookup(word);
        if (b.kind == BindingKind::Type) return b.id;
        if (b.kind != BindingKind::None) throw std::runtime_error("expected type binder");
        AbiBuiltinTypeKind builtin = abi_builtin_type_kind(word, nullptr);
        return builtin == ABI_BUILTIN_TYPE_NONE ? g.path(word) : g.builtin(builtin);
    }
    std::string op = word.substr(0, colon), rest = word.substr(colon + 1);
    if (op == "named" || op == "name") return g.path(rest);
    if (++depth > 1024) throw std::runtime_error("ABI type nesting limit exceeded");
    Id result = 0;
    if (op == "array" || op == "vector") {
        std::size_t split = rest.find(':');
        if (split == std::string::npos) throw std::runtime_error("missing array element type");
        auto bound = index_value(rest.substr(0, split));
        Id element = compact(rest.substr(split + 1));
        result = g.make(op == "array" ? Kind::Array : Kind::Vector, element, 0, 0, bound);
    } else if (op == "memberptr") {
        std::size_t split = 0;
        for (; split < rest.size(); ++split) {
            if (rest[split] != ':') continue;
            if (split + 1 < rest.size() && rest[split + 1] == ':') { ++split; continue; }
            break;
        }
        if (split == rest.size()) throw std::runtime_error("missing member pointer operand");
        Id owner = compact(rest.substr(0, split));
        Id member = compact(rest.substr(split + 1));
        result = g.make(Kind::MemberPointer, owner, member);
    } else {
        Id child = compact(rest);
        if (op == "ptr") result = g.make(Kind::Pointer, child);
        else if (op == "ref") result = g.make(Kind::Reference, child);
        else if (op == "rref") result = g.make(Kind::RvalueReference, child);
        else if (op == "const" || op == "volatile") result = g.cv(child, op == "const" ? 1 : 2);
        else if (op == "pack") result = g.make(Kind::Pack, child);
        else throw std::runtime_error("unknown compact ABI type constructor: " + op);
    }
    --depth; return result;
}
Id FactReader::type(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "name" || op == "named") return g.path(take(w, p));
    if (op == "template-param" || op == "template-param-subst" || op == "template-param-template") {
        auto index = index_value(take(w, p));
        if (index > UINT32_MAX) throw std::runtime_error("ABI parameter index too large");
        Id param = g.make(Kind::Parameter, index, op == "template-param-subst");
        if (op != "template-param-template") return param;
        return g.make(Kind::Template, param, 0, 0, 0, refs(w, p, BindingKind::Argument));
    }
    if (op == "ptr" || op == "ref" || op == "rref" || op == "const" || op == "volatile" || op == "pack") {
        if (++depth > 1024) throw std::runtime_error("ABI type nesting limit exceeded");
        Id child = type(w, p); --depth;
        if (op == "const" || op == "volatile") return g.cv(child, op == "const" ? 1 : 2);
        Kind kind = op == "ptr" ? Kind::Pointer : op == "ref" ? Kind::Reference :
            op == "rref" ? Kind::RvalueReference : Kind::Pack;
        return g.make(kind, child);
    }
    if (op == "vendor") {
        Id name = g.string(take(w, p)); Id child = type(w, p);
        return g.make(Kind::Vendor, child, name);
    }
    if (op == "tagged") {
        Id child = type(w, p); std::vector<Id> tags;
        while (p < w.size()) tags.push_back(g.string(take(w, p)));
        std::sort(tags.begin(), tags.end(), [this](Id a, Id b) { return g.spelling(a) < g.spelling(b); });
        tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
        return g.make(Kind::Tagged, child, 0, 0, 0, tags);
    }
    if (op == "template" || op == "std-template") {
        Id prefix;
        bool complete = false;
        if (op == "std-template") {
            auto code = abi_standard_substitution_kind(take(w, p));
            complete = boolean(take(w, p)); take(w, p); // descriptive source path
            prefix = g.make(Kind::Standard, code);
        } else prefix = g.path(take(w, p));
        auto args = refs(w, p, BindingKind::Argument);
        return complete ? prefix : g.make(Kind::Template, prefix, 0, 0, 0, args);
    }
    if (op == "member" || op == "member-template" || op == "member-pointer") {
        Id owner = type(w, p);
        if (op == "member-pointer") { Id member = type(w, p); return g.make(Kind::MemberPointer, owner, member); }
        Id name = g.name(owner, take(w, p));
        if (op == "member") return name;
        return g.make(Kind::Template, name, 0, 0, 0, refs(w, p, BindingKind::Argument));
    }
    if (op == "builtin-transform") {
        Id name = g.string(take(w, p)); std::vector<Id> types;
        while (p < w.size()) types.push_back(type(w, p));
        return g.make(Kind::Transform, 0, name, 0, 0, types);
    }
    if (op == "function-type" || op == "function-type-variadic") {
        Id result = type(w, p); std::vector<Id> params;
        while (p < w.size()) params.push_back(type(w, p));
        return g.make(Kind::FunctionType, result, 0, op == "function-type-variadic", 0, params);
    }
    if (op == "array" || op == "vector") {
        auto bound = index_value(take(w, p)); Id element = type(w, p);
        return g.make(op == "array" ? Kind::Array : Kind::Vector, element, 0, 0, bound);
    }
    if (op == "decltype") return g.make(Kind::Decltype, reference(take(w, p), BindingKind::Expression));
    if (op == "local-type" || op == "lambda-closure") {
        Id ctx = reference(take(w, p), BindingKind::Context);
        Id name = op == "local-type" ? g.string(take(w, p)) : 0;
        auto ordinal = index_value(take(w, p)); std::vector<Id> params;
        while (p < w.size()) params.push_back(type(w, p));
        return g.make(op == "local-type" ? Kind::Local : Kind::Lambda, ctx, name, 0, ordinal, params);
    }
    if (op == "namespace-lambda") {
        std::string name = take(w, p); Id parent = 0;
        while (p < w.size()) parent = g.name(parent, take(w, p));
        return g.name(parent, name);
    }
    return compact(op);
}
} // namespace abi_mangle
