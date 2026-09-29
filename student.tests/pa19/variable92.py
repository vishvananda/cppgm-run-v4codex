#!/usr/bin/env python3
"""Variable-template declaration, value and storage composition: CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD = {
 'outer_inner_values': 'template<int N>struct O{template<int M>static constexpr int v=N+M;};static_assert(O<3>::v<4> == 7,"");static_assert(O<9>::v<2> == 11,"");int main(){return O<3>::v<4> != 7;}',
 'inner_default': 'template<int N>struct O{template<int M=N>static constexpr int v=N+M;};static_assert(O<3>::v<> == 6,"");static_assert(O<9>::v<> == 18,"");int main(){}',
 'type_default': 'template<class T>struct O{template<class U=T>static constexpr int v=sizeof(T)+sizeof(U);};static_assert(O<int>::v<> == 8,"");static_assert(O<long>::v<char> == 9,"");int main(){}',
 'inner_pack': 'template<int N>struct O{template<class...T>static constexpr int v=N+sizeof...(T);};static_assert(O<3>::v<> == 3,"");static_assert(O<4>::v<int,long> == 6,"");int main(){}',
 'outer_pack': 'template<class...T>struct O{template<class...U>static constexpr int v=sizeof...(T)+sizeof...(U);};static_assert(O<int,long>::v<char> == 3,"");static_assert(O<>::v<> == 0,"");int main(){}',
 'dormant_outer_initializer': 'template<class T>struct O{template<class U>static constexpr int v=T::missing;};O<int> o;static_assert(sizeof(O<int>::v<long>)==sizeof(int),"");int main(){}',
 'dormant_initializer': 'template<class T>struct O{template<class U>static constexpr int v=U::missing;};O<int> o;int main(){}',
 'unevaluated_initializer': 'template<class T>struct O{template<class U>static constexpr int v=U::missing;};static_assert(sizeof(O<int>::v<long>)==sizeof(int),"");int main(){}',
 'namespace_unevaluated': 'template<class T>constexpr int v=T::missing;static_assert(sizeof(v<int>)==sizeof(int),"");int main(){}',
 'qualified_unevaluated': 'template<class T>struct O{template<class U>static constexpr int v=U::missing;};template<class T>using R=decltype(T::template v<int>);R<O<int>> n=3;int main(){return n!=3;}',
 'partial_selection': 'template<class T>constexpr int v=1;template<class T>constexpr int v<T* > =2;static_assert(v<int> == 1,"");static_assert(v<int*> == 2,"");int main(){return v<int*> != 2;}',
 'scalar_identity': 'template<int N>struct O{template<int M>static constexpr int v=N+M;};const int* a(){return &O<3>::v<4>;}const int* b(){return &O<3>::v<4>;}int main(){return a()!=b()||*a()!=7||a()==&O<4>::v<3>;}',
 'object_braces': 'struct X{int n;};template<class>struct O{template<int N>static constexpr X v={N};};int main(){return O<int>::v<7>.n!=7;}',
 'object_conversion': 'struct X{int n;constexpr X(int v):n(v){}};template<class>struct O{template<int N>static constexpr X v=N;};int main(){return O<int>::v<7>.n!=7;}',
 'scalar_conversion': 'struct X{constexpr operator int()const{return 7;}};template<class>struct O{template<int N>static constexpr int v=X();};int main(){return O<int>::v<3>!=7;}',
 'object_value': 'struct X{int n;constexpr X(int v):n(v){}};template<int N>struct O{template<int M>static constexpr X v=X(N+M);};int main(){X x=O<3>::v<4>;return x.n!=7;}',
 'object_identity': 'struct X{int n;constexpr X(int v):n(v){}};template<int N>struct O{template<int M>static constexpr X v=X(N+M);};const X* a(){return &O<3>::v<4>;}const X* b(){return &O<3>::v<4>;}int main(){return a()!=b()||a()->n!=7||a()==&O<4>::v<3>;}',
 'object_static_assert': 'struct X{int n;constexpr X(int v):n(v){}};template<int N>struct O{template<int M>static constexpr X v=X(N+M);};static_assert(O<3>::v<4>.n==7,"");int main(){}',
 'object_default_result': 'template<int N>struct O{int n;constexpr O(int v):n(v){} template<int M>static constexpr O make(){return O(N+M);} template<int M,class U=decltype(make<M>())>static constexpr U v=make<M>();};int main(){O<3>x=O<3>::v<4>;return x.n!=7;}',
 'default_sfinae': 'template<class...>struct V{typedef void type;};template<class...T>using Void=typename V<T...>::type;template<class T>struct O{template<class U,class R=typename U::type>static constexpr int v=sizeof(R);};template<class T,class=void>struct Has{static const int n=0;};template<class T>struct Has<T,Void<decltype(O<int>::template v<T>)>>{static const int n=1;};struct Yes{typedef int type;};static_assert(Has<int>::n==0,"");static_assert(Has<Yes>::n==1,"");int main(){}',
 'qualified_default_sfinae': 'template<class...>struct V{typedef void type;};template<class...T>using Void=typename V<T...>::type;template<class T>struct O{template<class U,class R=typename U::type>static constexpr int v=sizeof(R);};template<class T,class U,class=void>struct Has{static const int n=0;};template<class T,class U>struct Has<T,U,Void<decltype(T::template v<U>)>>{static const int n=1;};struct Yes{typedef int type;};static_assert(Has<O<int>,int>::n==0,"");static_assert(Has<O<int>,Yes>::n==1,"");int main(){}',
 'leaf_boolean': 'template<class T>struct Is{static const bool value=false;};template<>struct Is<int>{static const bool value=true;};template<int>struct O{template<class T>static constexpr bool v=Is<T>::value;};template<bool,class T=void>struct Enable{};template<class T>struct Enable<true,T>{typedef T type;};template<class O,class T,class=void>struct Has{static const int n=0;};template<class O,class T>struct Has<O,T,typename Enable<O::template v<T>>::type>{static const int n=1;};static_assert(Has<O<1>,int>::n==1,"");static_assert(Has<O<1>,long>::n==0,"");int main(){}',
}
runner.BAD = {
 'demand_invalid_initializer': 'template<class T>struct O{template<class U>static constexpr int v=U::missing;};int main(){return O<int>::v<long>;}',
 'demand_invalid_default': 'template<class T>struct O{template<class U,class R=typename U::type>static constexpr int v=sizeof(R);};int main(){return O<int>::v<long>;}',
 'wrong_value': 'template<int N>struct O{template<int M>static constexpr int v=N+M;};static_assert(O<3>::v<4> == 8,"");',
 'explicit_copy_initializer': 'struct X{int n;explicit constexpr X(int v):n(v){}};template<class>struct O{template<int N>static constexpr X v=N;};int main(){return O<int>::v<7>.n;}',
 'deleted_copy_initializer': 'struct X{int n;constexpr X(int v):n(v){} X(const X&)=delete;};template<class>struct O{template<int N>static constexpr X v=X(N);};int main(){return O<int>::v<7>.n;}',
 'bad_class_initializer': 'struct X{X(int){}};template<class>struct O{template<int N>static constexpr X v=X(N);};int main(){return O<int>::v<3>,0;}',
 'recursive_initializer': 'template<class T>struct O{template<class U>static constexpr int v=O<T>::template v<U>;};int main(){return O<int>::v<long>;}',
}
if __name__=='__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
