#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::layout_lifecycle(EntityId cls)
{
    auto info = entities[cls].class_info;
    auto& facts = class_facts[info];
    if (dynamic_class(cls)) {
        auto& model = virtual_classes[facts.virtual_info];
        for (unsigned j = 0; j < model.views.size(); ++j)
            if (model.views[j].store) model.store_order.push_back(j);
        if (virtual_base_count(cls))
            std::sort(model.store_order.begin(),model.store_order.end(),[&](unsigned a, unsigned b) {
                return model.views[a].offset < model.views[b].offset;
            });

    }
    facts.lifecycle_begin = lifecycle_bases.size();
    for (unsigned j = 0; j < virtual_base_count(cls); ++j) {
        auto base = virtual_base_type(cls,j);
        lifecycle_bases.push_back({base,virtual_base_offset(cls,base),0,true});
    }
    unsigned cursor = virtual_base_count(cls) ? 1 : 0;
    for (auto b = facts.first_base; b; b = bases[b].next) {
        if (bases[b].virtual_base) continue;
        auto base = bases[b].base;
        auto count = vtt_base_size(base);
        lifecycle_bases.push_back({base,bases[b].offset,count ? cursor : 0,false});
        cursor += count;
    }
    facts.lifecycle_count = lifecycle_bases.size()-facts.lifecycle_begin;
    if (!virtual_base_count(cls)) return;
    facts.vtt_secondary = cursor;
    auto& model = virtual_classes[facts.virtual_info];
    if (host_abi) {
        Index views, visited, physical;
        model.store_order.clear();
        for (auto& view : model.views) view.store = false;
        for (unsigned j = 0; j < model.views.size(); ++j)
            views.put(key(model.views[j].subobject,model.views[j].type),j+1);
        struct Visit { unsigned edge, object; bool primary; };
        std::vector<Visit> pending;
        auto push = [&](EntityId owner, unsigned object) {
            std::vector<Visit> children;
            auto primary = class_facts[entities[owner].class_info].primary_base;
            for (auto b = class_facts[entities[owner].class_info].first_base; b; b = bases[b].next) {
                if (!dynamic_class(bases[b].base)) continue;
                children.push_back({b,compose_subobject(object,prefix_subobject(b,0)),b == primary && !bases[b].virtual_base});
            }
            pending.insert(pending.end(),children.rbegin(),children.rend());
        };
        push(cls,0);
        while (!pending.empty()) {
            auto item = pending.back(); pending.pop_back();
            auto base = bases[item.edge].base;
            auto identity = key(item.object,base);
            if (visited.get(identity)) continue;
            visited.put(identity,1); ++virtual_base_work;
            auto index = views.get(identity);
            if (index) {
                auto& view = model.views[index-1];
                if (view.offset && !physical.get(view.offset)) {
                    physical.put(view.offset,index); view.store = true; model.store_order.push_back(index-1);
                }
            }
            if (!item.primary && (subobjects[item.object].anchor || virtual_base_count(base))) {
                if (!index) throw std::logic_error("missing VTT subobject view");
                model.views[index-1].vtt_index = cursor++; model.vtt_order.push_back(index-1);
            }
            push(base,item.object);
        }
        std::sort(model.store_order.begin(),model.store_order.end(),[&](unsigned a, unsigned b) {
            return model.views[a].offset < model.views[b].offset;
        });
        auto end = model.address_point+std::uint64_t(model.primary_count)*8;
        for (unsigned j : model.store_order) {
            auto& view = model.views[j]; view.group_address_point = end+view.address_point;
            end = view.group_address_point+std::uint64_t(view.count)*8;
        }
        for (auto& view : model.views) if (!view.store)
            view.group_address_point = view.offset ? model.views[physical.get(view.offset)-1].group_address_point : model.address_point;
    } else for (const auto& view : model.views) cursor += view.store;
    facts.vtt_base_size = cursor;
    // Complete-object VTTs append virtual sub-VTTs once. Sub-VTTs omit
    // these tails, so a shared diamond never expands all inheritance paths.
    for (unsigned j = 0; j < virtual_base_count(cls); ++j) {
        auto position = host_abi ? virtual_base_index.get(key(cls,model.base_order[j]))-1-facts.virtual_bases_begin : j;
        auto& base = lifecycle_bases[facts.lifecycle_begin+position];
        auto count = vtt_base_size(base.type);
        if (count) { base.vtt = cursor; cursor += count; }
    }
    facts.vtt_size = cursor;
}
} }
