#!/usr/bin/env python3
"""Immediate substitution controls: CC WORK; execute successes via PA8 backend."""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
    'explicit_default_expression': 'template<class T>T&& val();template<class T,class=decltype(val<T>()++)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<int&>(0)!=1||f<const int&>(0)!=2||f<void*&>(0)!=2;}',
    'explicit_result_expression': 'template<class T>T&& val();template<class T>auto f(int)->decltype(val<T>()++,int()){return 1;}template<class>int f(...){return 2;}int main(){return f<int&>(0)!=1||f<const int&>(0)!=2;}',
    'deduced_result_expression': 'template<class T>auto f(T&t,int)->decltype(t++,int()){return 1;}int f(...){return 2;}int main(){int i=0;const int j=0;return f(i,0)!=1||f(j,0)!=2;}',
    'explicit_alias_failure': 'template<class T>using ptr=T*;template<class T>ptr<T> f(int){return 0;}template<class>int f(...){return 7;}int main(){return f<int>(0)!=0||f<int&>(0)!=7;}',
    'explicit_missing_result': 'struct A{typedef int type;};template<class T>typename T::type f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<A>(0)!=1||f<int>(0)!=2;}',
    'failed_query_reuse': 'template<class T>T&& val();template<class T,class=decltype(val<T>()++)>int f(int){return 1;}template<class>int f(...){return 2;}int main(){return f<const int&>(0)+f<const int&>(0)+f<int&>(0)!=5;}',
    'unselected_body': 'template<class T>typename T::type f(int){return T::missing;}template<class>int f(...){return 2;}int main(){return f<int>(0)!=2;}',
}
runner.BAD = {
    'class_side_effect': 'template<class T>struct A{typedef typename T::missing type;};template<class T>typename A<T>::type f(int);template<class>int f(...);int main(){return f<int>(0);}',
    'selected_body': 'struct A{typedef int type;};template<class T>typename T::type f(int){return T::missing;}template<class>int f(...){return 2;}int main(){return f<A>(0);}',
    'fixed_invalid_expression': 'template<class T>auto f(T)->decltype(*1);int main(){}',
    'no_surviving_candidate': 'template<class T>typename T::missing f(int);int main(){return f<int>(0);}',
}
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
