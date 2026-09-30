#include "native/encoding.h"
#include "support/id_index.h"
namespace native {
namespace {
void uleb(std::vector<unsigned char>& out, std::uint64_t n) {
    do { auto b = n&127; n >>= 7; out.push_back(b|(n ? 128 : 0)); } while (n);
}
void sleb(std::vector<unsigned char>& out, std::int64_t n) {
    bool more;
    do { unsigned b = n&127; n >>= 7; more = !((n == 0 && !(b&64)) || (n == -1 && (b&64))); out.push_back(b|(more ? 128 : 0)); } while (more);
}
}
void Encoder::host_tables()
{
    auto& f = *function;
    if (!host_landings.empty()) {
        std::vector<unsigned char> actions, sites;
        std::vector<unsigned> action(f.blocks.size());
        cppgm::IdIndex blocks;
        for (unsigned b = 0; b < f.blocks.size(); ++b) blocks.put(f.blocks[b].id,b+1);
        for (unsigned b : host_landings) {
            const auto& block = f.blocks[b];
            if (!block.instructions.count || f.instructions[block.instructions.begin].op != Op::EhDispatch) continue;
            const auto& h = f.exception_handlers[f.instructions[block.instructions.begin].args[0].bits];
            if (!h.clauses.count) continue;
            action[b] = actions.size()+1;
            for (unsigned n = h.clauses.begin; n < h.clauses.end(); ++n) {
                sleb(actions,f.exception_clauses[n].host_selector);
                sleb(actions,n+1 != h.clauses.end() || h.cleanup ? 1 : 0);
            }
            if (h.cleanup) { sleb(actions,0); sleb(actions,0); }
        }
        // Sites arrive in final address order. Merge only identical landing and
        // action continuations. Null sites are barriers, and gaps stay sparse.
        std::vector<HostSite> compact;
        for (auto s : host_sites) {
            if (!compact.empty() && compact.back().handler == s.handler) compact.back().end = s.end;
            else compact.push_back(s);
        }
        for (auto s : compact) {
            auto b = s.handler ? blocks.get(s.handler)-1 : 0;
            uleb(sites,s.begin-unwind_record.begin); uleb(sites,s.end-s.begin);
            uleb(sites,s.handler ? host_landing_offsets[b]-unwind_record.begin : 0);
            uleb(sites,s.handler ? action[b] : 0);
        }
        auto& out = image.lsda;
        unwind_record.lsda = out.size(); unwind_record.has_lsda = true;
        out.push_back(0xff); // LPStart is the FDE start
        out.push_back(f.host_types.empty() ? 0xff : 0x9b); // indirect pcrel sdata4
        std::vector<unsigned char> body;
        body.push_back(1); uleb(body,sites.size()); body.insert(body.end(),sites.begin(),sites.end());
        body.insert(body.end(),actions.begin(),actions.end());
        if (!f.host_types.empty()) uleb(out,body.size()+4*f.host_types.size());
        out.insert(out.end(),body.begin(),body.end());
        for (auto it = f.host_types.rbegin(); it != f.host_types.rend(); ++it) {
            if (*it) { Fixup fix; fix.offset = out.size(); fix.symbol = it->index; image.lsda_fixups.push_back(fix); }
            out.insert(out.end(),4,0);
        }
    }
    unwind_record.end = code.size(); image.unwind.push_back(std::move(unwind_record));
}
} // namespace native
