#pragma once
#include "preprocess/source.h"
namespace cppgm {
enum class AtomicOp : unsigned char { None, Load, Store, Exchange, Compare, Add, Sub, And, Or, Xor, Nand, Clear, TestSet, ThreadFence, SignalFence, LockFree, Init };
enum class AtomicForm : unsigned char { Scalar, Generic, Sync, SyncValue, SyncBool, Always, C11 };
// Immutable vocabulary; the packed descriptor is also the semantic family key.
struct AtomicBuiltin {
    AtomicOp op = AtomicOp::None;
    AtomicForm form = AtomicForm::Scalar;
    bool updated = false;
    AtomicBuiltin() {}
    AtomicBuiltin(AtomicOp o, AtomicForm f, bool u = false) : op(o), form(f), updated(u) {}
    explicit AtomicBuiltin(unsigned code) : op(AtomicOp(code & 31)), form(AtomicForm((code >> 5) & 7)), updated(code & 256) {}
    unsigned code() const { return unsigned(op) | (unsigned(form) << 5) | (unsigned(updated) << 8); }
    bool sync() const { return form == AtomicForm::Sync || form == AtomicForm::SyncValue || form == AtomicForm::SyncBool; }
    bool fence() const { return op == AtomicOp::ThreadFence || op == AtomicOp::SignalFence; }
    bool arithmetic() const { return op >= AtomicOp::Add && op <= AtomicOp::Nand; }
    unsigned arity() const {
        if (fence()) return sync() ? 0 : 1;
        if (op == AtomicOp::LockFree) return form == AtomicForm::C11 ? 1 : 2;
        if (op == AtomicOp::Init) return 2;
        if (op == AtomicOp::Compare) return sync() ? 3 : form == AtomicForm::C11 ? 5 : 6;
        if (op == AtomicOp::Clear) return sync() ? 1 : 2;
        if (op == AtomicOp::TestSet) return 2;
        unsigned n = op == AtomicOp::Load ? 2 : 3;
        if (sync()) --n;
        if (form == AtomicForm::Generic && (op == AtomicOp::Load || op == AtomicOp::Exchange)) ++n;
        return n;
    }
};
inline AtomicBuiltin atomic_builtin(TextView name)
{
    AtomicForm form;
    unsigned prefix;
    if (name.size > 9 && TextView(name.data,9).equals("__atomic_")) { prefix = 9; form = AtomicForm::Scalar; }
    else if (name.size > 7 && TextView(name.data,7).equals("__sync_")) { prefix = 7; form = AtomicForm::Sync; }
    else if (name.size > 13 && TextView(name.data,13).equals("__c11_atomic_")) { prefix = 13; form = AtomicForm::C11; }
    else return {};
    auto suffix = TextView(name.data+prefix,name.size-prefix);
    struct Entry { const char* name; AtomicOp op; bool updated; };
    static const Entry arithmetic[] = {
        {"fetch_add",AtomicOp::Add,false},{"add_fetch",AtomicOp::Add,true},
        {"fetch_sub",AtomicOp::Sub,false},{"sub_fetch",AtomicOp::Sub,true},
        {"fetch_and",AtomicOp::And,false},{"and_fetch",AtomicOp::And,true},
        {"fetch_or",AtomicOp::Or,false},{"or_fetch",AtomicOp::Or,true},
        {"fetch_xor",AtomicOp::Xor,false},{"xor_fetch",AtomicOp::Xor,true},
        {"fetch_nand",AtomicOp::Nand,false},{"nand_fetch",AtomicOp::Nand,true}
    };
    if (form == AtomicForm::Sync) {
        static const Entry sync[] = {
            {"fetch_and_add",AtomicOp::Add,false},{"add_and_fetch",AtomicOp::Add,true},
            {"fetch_and_sub",AtomicOp::Sub,false},{"sub_and_fetch",AtomicOp::Sub,true},
            {"fetch_and_and",AtomicOp::And,false},{"and_and_fetch",AtomicOp::And,true},
            {"fetch_and_or",AtomicOp::Or,false},{"or_and_fetch",AtomicOp::Or,true},
            {"fetch_and_xor",AtomicOp::Xor,false},{"xor_and_fetch",AtomicOp::Xor,true},
            {"fetch_and_nand",AtomicOp::Nand,false},{"nand_and_fetch",AtomicOp::Nand,true},
            {"lock_test_and_set",AtomicOp::Exchange,false},{"lock_release",AtomicOp::Clear,false},
            {"synchronize",AtomicOp::ThreadFence,false}
        };
        for (auto e : sync) if (suffix.equals(e.name)) return {e.op,form,e.updated};
        if (suffix.equals("val_compare_and_swap")) return {AtomicOp::Compare,AtomicForm::SyncValue};
        if (suffix.equals("bool_compare_and_swap")) return {AtomicOp::Compare,AtomicForm::SyncBool};
        return {};
    }
    for (auto e : arithmetic) if (suffix.equals(e.name) &&
        (form != AtomicForm::C11 || (!e.updated && e.op != AtomicOp::Nand))) return {e.op,form,e.updated};
    if (suffix.equals("thread_fence")) return {AtomicOp::ThreadFence,form};
    if (suffix.equals("signal_fence")) return {AtomicOp::SignalFence,form};
    if (suffix.equals("is_lock_free")) return {AtomicOp::LockFree,form};
    if (form == AtomicForm::C11) {
        if (suffix.equals("init")) return {AtomicOp::Init,form};
        if (suffix.equals("load")) return {AtomicOp::Load,form};
        if (suffix.equals("store")) return {AtomicOp::Store,form};
        if (suffix.equals("exchange")) return {AtomicOp::Exchange,form};
        if (suffix.equals("compare_exchange_strong") || suffix.equals("compare_exchange_weak")) return {AtomicOp::Compare,form};
    } else {
        if (suffix.equals("load_n")) return {AtomicOp::Load,form};
        if (suffix.equals("store_n")) return {AtomicOp::Store,form};
        if (suffix.equals("exchange_n")) return {AtomicOp::Exchange,form};
        if (suffix.equals("compare_exchange_n")) return {AtomicOp::Compare,form};
        if (suffix.equals("load")) return {AtomicOp::Load,AtomicForm::Generic};
        if (suffix.equals("store")) return {AtomicOp::Store,AtomicForm::Generic};
        if (suffix.equals("exchange")) return {AtomicOp::Exchange,AtomicForm::Generic};
        if (suffix.equals("compare_exchange")) return {AtomicOp::Compare,AtomicForm::Generic};
        if (suffix.equals("test_and_set")) return {AtomicOp::TestSet,form};
        if (suffix.equals("clear")) return {AtomicOp::Clear,form};
        if (suffix.equals("always_lock_free")) return {AtomicOp::LockFree,AtomicForm::Always};
    }
    return {};
}
}
