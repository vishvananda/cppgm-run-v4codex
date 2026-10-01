// C++11 [class]/6, [dcl.fct.def.default]/4, [class.copy]/12,25.
struct DeletedCopy {
 DeletedCopy() = default;
 DeletedCopy(const DeletedCopy&) = delete;
 DeletedCopy& operator=(const DeletedCopy&) = delete;
};
static_assert(__is_trivially_copyable(DeletedCopy),"deleted is not user-provided");
static_assert(__is_trivial(DeletedCopy),"trivial default constructor");
static_assert(__is_pod(DeletedCopy),"empty standard-layout trivial class");
static_assert(!__is_constructible(DeletedCopy,const DeletedCopy&),"copy is unusable");
static_assert(!__is_assignable(DeletedCopy&,const DeletedCopy&),"assignment is unusable");
struct NonTrivial { NonTrivial(const NonTrivial&){}; };
static_assert(!__is_trivially_copyable(NonTrivial),"user-provided copy differs");
int main(){return 0;}
