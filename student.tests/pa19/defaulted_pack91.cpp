// N3485 [temp.arg]/4, [temp.deduct.type]/9, [expr.sizeof]/5.
struct null_type {};
template<class A=null_type, class B=null_type, class C=null_type,
         class D=null_type, class E=null_type, class F=null_type,
         class G=null_type, class H=null_type, class I=null_type,
         class J=null_type> struct tuple {};
template<class, class> struct same;
template<class T> struct same<T,T> {};
same<tuple<int,int,int>, tuple<int,int,int,null_type,null_type,null_type,
     null_type,null_type,null_type,null_type>> identity;
template<class T, class...Ts> constexpr unsigned count(const tuple<T,Ts...>&) {
    return sizeof...(Ts);
}
static_assert(count(tuple<int,int,int>()) == 9, "defaults belong to the type");
static_assert(count(tuple<>()) == 9, "all defaults belong to the type");
int main() { return count(tuple<int,int,int>()) != 9; }
