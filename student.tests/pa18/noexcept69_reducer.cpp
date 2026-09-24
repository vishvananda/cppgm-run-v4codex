// N3485 [expr.pseudo]/1 preserves evaluation of the receiver; its call may throw.
using I = int;
I* value();
static_assert(noexcept(value()->~I()), "receiver may throw");
