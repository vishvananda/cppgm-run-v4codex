#!/usr/bin/env python3
"""Template static storage definition timing, publication and demanded counterparts."""
from pathlib import Path
import sys
import ordering_controls as runner
runner.GOOD={
 'unused_const_invalid': 'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;int main(){X<int>x;return 0;}',
 'unused_const_effect': 'int hits;int f(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=f();int main(){X<int>x;return hits;}',
 'unused_object_effect': 'int hits;struct E{E(){++hits;}~E(){++hits;}};template<class T>struct X{static const E n;};template<class T>const E X<T>::n;int main(){X<int>x;return hits;}',
 'used_const_effect': 'int hits;int f(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=f();int main(){return X<int>::n!=1||hits!=1;}',
 'used_object_effect': 'int hits;struct E{int n;E():n(++hits){}};template<class T>struct X{static const E n;};template<class T>const E X<T>::n;int main(){return X<int>::n.n!=1||hits!=1;}',
 'unevaluated_static': 'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;int main(){static_assert(sizeof(X<int>::n)==sizeof(int),"");using T=decltype(X<int>::n);return sizeof(T)!=sizeof(int);}',
 'select_specialization': 'int hits;int f(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=f();template<>const int X<char>::n=7;int main(){X<int>x;return X<char>::n!=7||hits;}',
 'explicit_member_definition': 'int hits;int f(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=f();template const int X<int>::n;int main(){return hits!=1||X<int>::n!=1;}',
 'explicit_class_definition': 'int hits;int f(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=f();template struct X<int>;int main(){return hits!=1||X<int>::n!=1;}',
 'dormant_namespace': 'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;X<int>x;int main(){}',
 'unused_late_definition': 'template<class T>struct X{static const int n;};int main(){X<int>x;}template<class T>const int X<T>::n=T::missing;',
 'constant_inclass': 'template<class T>struct X{static const int n=3;};template<class T>const int X<T>::n;int main(){return X<int>::n!=3;}',
 'reference_storage': 'int v=3;template<class T>struct X{static int&r;};template<class T>int&X<T>::r=v;int main(){X<int>::r=7;return v!=7||&X<int>::r!=&v;}',
 'transitive_demand': 'template<class T>struct X{static const int n;static const int unused;};template<class T>const int X<T>::n=3;template<class T>const int X<T>::unused=T::missing;template<class T>struct Y{static const int n;};template<class T>const int Y<T>::n=X<T>::n;int main(){return Y<int>::n!=3;}',
}
runner.BAD={
 'used_invalid_const':'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;int main(){return X<int>::n;}',
 'explicit_invalid_const':'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;template const int X<int>::n;int main(){}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
