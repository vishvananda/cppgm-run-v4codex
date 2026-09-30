#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::layout_lifecycle(EntityId cls)
{
    auto info = entities[cls].class_info;
    auto& facts = class_facts[info];
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
    for (const auto& view : virtual_class(cls).views) cursor += view.store;
    facts.vtt_base_size = cursor;
    // Complete-object VTTs append virtual sub-VTTs once. Sub-VTTs omit
    // these tails, so a shared diamond never expands all inheritance paths.
    for (unsigned j = 0; j < virtual_base_count(cls); ++j) {
        auto& base = lifecycle_bases[facts.lifecycle_begin+j];
        auto count = vtt_base_size(base.type);
        if (count) { base.vtt = cursor; cursor += count; }
    }
    facts.vtt_size = cursor;
}
} }
