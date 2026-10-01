#include "semantic/analyzer.h"
#include <algorithm>
namespace cppgm { namespace semantic {
void Analyzer::constant_overlay_dependencies(std::uint32_t storage, std::uint32_t id, bool attach)
{
    auto& owner = constant_storage[storage];
    auto& entries = owner.frame->overlays;
    auto& entry = entries[id];
    if (entry.dependency_previous) entries[entry.dependency_previous].dependency_next = entry.dependency_next;
    else if (owner.address_overlays == id) owner.address_overlays = entry.dependency_next;
    if (entry.dependency_next) entries[entry.dependency_next].dependency_previous = entry.dependency_previous;
    entry.dependency_previous = entry.dependency_next = 0;
    if (!attach || !entry.value.valid) return;
    auto value = entry.value; auto kind = types[value.type].kind;
    bool addresses = (kind == TypeKind::Pointer || kind == TypeKind::LRef || kind == TypeKind::RRef) && value.bits;
    if (class_value(value.type) || kind == TypeKind::Array) addresses = evaluated_objects[value.bits].address_count;
    if (!addresses) return;
    entry.dependency_next = owner.address_overlays;
    if (entry.dependency_next) entries[entry.dependency_next].dependency_previous = id;
    owner.address_overlays = id;
}
void Analyzer::constant_write(std::uint32_t address, Constant value)
{
    auto storage = constant_addresses[address].storage;
    auto& frame = *constant_storage[storage].frame;
    // Sparse paths belong to the activation owning the storage, even when a
    // nested call writes through a reference. Invalidate only ancestors.
    std::vector<std::uint32_t> missing;
    for (auto p = address; p && !frame.overlay_index.get(p); p = constant_addresses[p].parent)
        missing.push_back(p);
    for (auto it = missing.rbegin(); it != missing.rend(); ++it) {
        auto parent = frame.overlay_index.get(constant_addresses[*it].parent);
        ConstantOverlay entry; entry.address = *it;
        entry.next = parent ? frame.overlays[parent].first : 0;
        auto id = frame.overlays.size(); frame.overlays.push_back(entry);
        frame.overlay_index.put(*it,id);
        if (parent) frame.overlays[parent].first = id;
    }
    auto id = frame.overlay_index.get(address);
    // Whole-subobject assignment retires its old descendants. Work is charged
    // to previously created dirty paths, never all subobjects or all storage.
    std::vector<std::uint32_t> retired;
    if (frame.overlays[id].first) retired.push_back(frame.overlays[id].first);
    while (!retired.empty()) {
        auto p = retired.back(); retired.pop_back(); auto entry = frame.overlays[p];
        constant_overlay_dependencies(storage,p,false);
        frame.overlay_index.put(entry.address,0);
        if (entry.first) retired.push_back(entry.first);
        if (entry.next) retired.push_back(entry.next);
    }
    frame.overlays[id].first = 0; frame.overlays[id].value = value; frame.overlays[id].dirty = false;
    constant_overlay_dependencies(storage,id,true);
    for (auto p = constant_addresses[address].parent; p; p = constant_addresses[p].parent) {
        auto entry = frame.overlay_index.get(p);
        if (frame.overlays[entry].dirty) break;
        frame.overlays[entry].dirty = true;
    }
    ++constant_storage[storage].version;
    if (!constant_addresses[address].parent) {
        constant_storage[storage].value = value;
        frame.values[constant_storage[storage].binding] = value;
    }
}
Constant Analyzer::constant_snapshot(std::uint32_t address, Constant value)
{
    if (!address) return value;
    auto a = constant_addresses[address]; auto storage = constant_storage[a.storage];
    if (!storage.frame) return value;
    auto& frame = *storage.frame;
    auto id = frame.overlay_index.get(address);
    if (!id || !frame.overlays[id].dirty) return value;
    // Scalar reads consult their own overlay in O(depth); only a requested
    // aggregate value freezes its changed subtree. Unchanged payloads are shared.
    std::vector<std::uint32_t> children;
    for (auto c = frame.overlays[id].first; c; c = frame.overlays[c].next)
        children.push_back(frame.overlays[c].address);
    std::sort(children.begin(),children.end(),[&](std::uint32_t x,std::uint32_t y) {
        return constant_addresses[x].selector < constant_addresses[y].selector;
    });
    auto object = evaluated_objects[value.bits];
    std::vector<EvaluatedPart> parts;
    if (types[value.type].kind == TypeKind::Array) {
        unsigned child = 0;
        for (unsigned i = 0; i < object.count; ++i) {
            auto part = evaluated_parts[object.first+i]; auto end = part.selector+part.count;
            while (child < children.size() && constant_addresses[children[child]].selector < end) {
                auto at = constant_addresses[children[child]].selector;
                if (part.selector < at) { auto prefix = part; prefix.count = at-part.selector; parts.push_back(prefix); }
                EvaluatedPart changed; changed.selector = at; changed.value = constant_read(children[child++]); parts.push_back(changed);
                part.selector = at+1; part.count = end-part.selector;
            }
            if (part.count) parts.push_back(part);
        }
    } else {
        Index changes;
        for (auto c : children) changes.put(constant_addresses[c].selector,c);
        for (unsigned i = 0; i < object.count; ++i) {
            auto part = evaluated_parts[object.first+i];
            if (auto c = changes.get(part.selector)) part.value = constant_read(c);
            parts.push_back(part);
        }
    }
    value = evaluated_object(value.type,parts);
    frame.overlays[id].value = value; frame.overlays[id].dirty = false;
    if (!a.parent) {
        constant_storage[a.storage].value = value;
        frame.values[storage.binding] = value;
    }
    return value;
}
} }
