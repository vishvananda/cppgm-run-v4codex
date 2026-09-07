#include "abi/itanium/fact_reader.h"
#include <algorithm>
#include <stdexcept>

namespace abi_mangle {
Id FactReader::compact(const std::string& word) {
    // Peel compact modifiers without copying each remaining suffix. A long
    // pointer/cv/array chain takes linear bytes and flat temporary storage.
    struct Modifier { Kind kind; std::uint64_t value; };
    std::vector<Modifier> modifiers;
    std::size_t pos = 0;
    for (;;) {
        std::size_t colon = word.find(':', pos);
        if (colon == std::string::npos || (colon + 1 < word.size() && word[colon + 1] == ':')) break;
        std::string op = word.substr(pos, colon - pos);
        Kind kind;
        std::uint64_t value = 0;
        if (op == "ptr") kind = Kind::Pointer;
        else if (op == "ref") kind = Kind::Reference;
        else if (op == "rref") kind = Kind::RvalueReference;
        else if (op == "pack") kind = Kind::Pack;
        else if (op == "const" || op == "volatile") { kind = Kind::Cv; value = op == "const" ? 1 : 2; }
        else if (op == "array" || op == "vector") {
            kind = op == "array" ? Kind::Array : Kind::Vector;
            std::size_t split = word.find(':', colon + 1);
            if (split == std::string::npos) throw std::runtime_error("missing array element type");
            value = index_value(word.substr(colon + 1, split - colon - 1)); colon = split;
        } else break;
        modifiers.push_back({kind, value}); pos = colon + 1;
    }
    std::string rest = word.substr(pos);
    Id result;
    if (rest.compare(0, 6, "named:") == 0) result = g.path(rest.substr(6));
    else if (rest.compare(0, 5, "name:") == 0) result = g.path(rest.substr(5));
    else if (rest.compare(0, 10, "memberptr:") == 0) {
        std::size_t split = 10;
        for (; split < rest.size(); ++split) {
            if (rest[split] != ':') continue;
            if (split + 1 < rest.size() && rest[split + 1] == ':') { ++split; continue; }
            break;
        }
        if (split == rest.size()) throw std::runtime_error("missing member pointer operand");
        if (++depth > 1024) throw std::runtime_error("ABI member pointer nesting limit exceeded");
        Id owner = compact(rest.substr(10, split - 10));
        Id member = compact(rest.substr(split + 1)); --depth;
        result = g.make(Kind::MemberPointer, owner, member);
    } else {
        for (std::size_t i = 0; i < rest.size(); ++i) {
            if (rest[i] != ':') continue;
            if (i + 1 == rest.size() || rest[i + 1] != ':')
                throw std::runtime_error("unknown compact ABI type constructor");
            ++i;
        }
        Binding b = lookup(rest);
        if (b.kind == BindingKind::Type) result = b.id;
        else if (b.kind != BindingKind::None) throw std::runtime_error("expected type binder");
        else {
            AbiBuiltinTypeKind builtin = abi_builtin_type_kind(rest, nullptr);
            result = builtin == ABI_BUILTIN_TYPE_NONE ? g.path(rest) : g.builtin(builtin);
        }
    }
    for (auto i = modifiers.rbegin(); i != modifiers.rend(); ++i) {
        if (i->kind == Kind::Cv) result = g.cv(result, i->value);
        else result = g.make(i->kind, result, 0, 0, i->value);
    }
    return result;
}
Id FactReader::type(const Words& w, std::size_t& p) {
    std::string op = take(w, p);
    if (op == "name" || op == "named") return g.path(take(w, p));
    if (op == "template-param" || op == "template-param-subst" || op == "template-param-template") {
        auto index = index_value(take(w, p));
        Id param = g.make(Kind::Parameter, 0, op == "template-param-subst", 0, index);
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
    if (op == "template-name") {
        Id prefix = type(w, p);
        return g.make(Kind::Template, prefix, 0, 0, 0, refs(w, p, BindingKind::Argument));
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
    if (op == "array-expression") {
        Id bound = reference(take(w, p), BindingKind::Expression); Id element = type(w, p);
        return g.make(Kind::Array, element, bound);
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
