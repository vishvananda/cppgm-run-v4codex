struct M { M(M&)=default; M(const M&) {} M& operator=(M&)=default; M& operator=(const M&){return *this;} };
struct C { M m; C(C&)=default; C& operator=(C&)=default; };
static_assert(__has_trivial_copy(C),"selected copy");
static_assert(__has_trivial_assign(C),"selected assignment");
int main(){return 0;}
struct DeletedDefault { DeletedDefault()=delete; };
static_assert(__has_trivial_constructor(DeletedDefault),"member triviality");
static_assert(__is_trivial(DeletedDefault),"triviality independent of usable default");
static_assert(!__is_constructible(DeletedDefault),"unusable default");
struct ConstMember { const M m; ConstMember(ConstMember&)=default; };
static_assert(!__has_trivial_copy(ConstMember),"const member selects nontrivial copy");
struct MutableMember { mutable M m; MutableMember(const MutableMember&)=default; };
static_assert(__has_trivial_copy(MutableMember),"mutable member selects nonconst copy");
struct PrivateMember { private: PrivateMember(const PrivateMember&)=default; };
struct DeletedOuter { PrivateMember m; };
static_assert(__has_trivial_copy(DeletedOuter),"deleted due to access can be structurally trivial");
static_assert(!__is_constructible(DeletedOuter,const DeletedOuter&),"still inaccessible subobject");
static_assert(__is_trivially_constructible(C,C&),"operation consumes same selected member");
static_assert(__has_trivial_copy(C),"prepared transfer does not invalidate structural fact");
