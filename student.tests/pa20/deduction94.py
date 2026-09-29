#!/usr/bin/env python3
"""PA20 placeholder controls. Run CC WORK; native successes and rejections."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'floating_return': 'auto f(){return 3.5;}int main(){return f()!=3.5||sizeof(f())!=sizeof(double);}',
 'multiple_returns': 'auto f(int n){if(n)return 3L;return 7L;}int main(){return f(0)!=7||f(1)!=3||sizeof(f(0))!=sizeof(long);}',
 'no_returns': 'int count;auto f(){++count;}int main(){f();return count!=1;}',
 'void_expression': 'int count;void g(){++count;}auto f(){return g();}int main(){f();return count!=1;}',
 'void_returns': 'int count;auto f(int n){if(n){++count;return;}return;}int main(){f(1);f(0);return count!=1;}',
 'recursive_after_deduction': 'auto f(int n){if(!n)return 1L;return n*f(n-1);}int main(){return f(5)!=120||sizeof(f(1))!=sizeof(long);}',
 'pointer_return': 'int value;auto* f(){return &value;}int main(){*f()=8;return value!=8;}',
 'reference_return': 'int value;auto& f(){return value;}int main(){f()=8;return value!=8;}',
 'forward_reference': 'int value;auto&& f(){return value;}int main(){f()=8;return value!=8;}',
 'cv_reference': 'const long value=8;const auto& f(){return value;}int main(){return &f()!=&value||f()!=8;}',
 'array_reference': 'int value[2]={3,4};auto& f(){return value;}int main(){f()[1]=9;return sizeof(f())!=sizeof(value)||value[1]!=9;}',
 'array_decay': 'int value[2]={3,4};auto f(){return value;}int main(){return f()!=value||f()[1]!=4;}',
 'function_decay': 'long g(){return 7;}auto f(){return g;}int main(){return f()()!=7;}',
 'member_lvalue': 'struct X{int n;auto&& f()&{return *this;}};int main(){X x;x.f().n=8;return x.n!=8;}',
 'member_rvalue': 'struct X{int n;X(int v):n(v){}auto&& f()&&{return static_cast<X&&>(*this);}};int main(){return X(8).f().n!=8;}',
 'member_later_definition': 'struct X{long f(){return g();}auto g(){return 7L;}};int main(){X x;return x.f()!=7||sizeof(x.g())!=sizeof(long);}',
 'out_of_class': 'struct X{auto f();};auto X::f(){return 7L;}int main(){X x;return x.f()!=7||sizeof(x.f())!=sizeof(long);}',
 'class_value_return': 'struct X{int n;};auto f(){return X{7};}int main(){return f().n!=7;}',
 'deduced_constexpr': 'constexpr auto f(){return 7L;}static_assert(f()==7,"");int main(){return sizeof(f())!=sizeof(long);}',
 'template_different_results': 'template<class T>constexpr auto f(T x){return x;}static_assert(f(3L)==3,"");int main(){return sizeof(f(1L))!=sizeof(long)||sizeof(f(2))!=sizeof(int)||f(3.5)!=3.5;}',
 'template_reference': 'template<class T>auto&& f(T&x){return x;}int main(){long n=1;f(n)=7;return n!=7||&f(n)!=&n;}',
 'template_member': 'template<class T>struct X{T n;auto& f(){return n;}};int main(){X<long>x;x.f()=7;return x.n!=7;}',
 'template_result_query': 'template<class T>auto f(T x){return x;}static_assert(sizeof(f(1L))==sizeof(long),"");int main(){return f(2L)!=2;}',
 'unselected_body': 'template<class T>auto f(T x){return x.missing;}long f(int){return 7;}int main(){return f(1)!=7;}',
 'condition_pointer': 'struct X{long n;};X v={7};X*get(){return &v;}int main(){if(auto p=get())return p->n!=7;return 1;}',
 'condition_template': 'template<class T>int f(T*p){if(auto q=p)return *q!=7;return 1;}int main(){long n=7;return f(&n);}',
 'cv_object': 'long f(){return 7;}int main(){const auto n=f();volatile auto m=f();return sizeof(n)!=sizeof(long)||n!=7||m!=7;}',
 'trailing_unchanged': 'auto f()->long{return 7;}int main(){return f()!=7||sizeof(f())!=sizeof(long);}',
 'template_address': 'template<class T>auto f(T x){return x;}int main(){long(*p)(long)=f;return p(7)!=7;}',
 'lambda_multiple_returns': 'int main(){auto f=[](int n){long x=3;if(n)return x;return 7L;};return f(0)!=7||f(1)!=3||sizeof(f(0))!=sizeof(long);}',
 'lambda_nested_returns': 'auto f(){auto g=[](){return 3L;};return g();}int main(){return f()!=3||sizeof(f())!=sizeof(long);}',
 'lambda_void': 'int main(){auto f=[](){};f();return 0;}',
 'template_lambda_return': 'template<class T>auto f(T x){auto g=[](T n){T r=n;return r;};return g(x);}int main(){return f(3L)!=3||sizeof(f(3L))!=sizeof(long);}',
}
runner.BAD = {
 'inconsistent_returns': 'auto f(int n){if(n)return 1;return 2.0;}int main(){}',
 'inconsistent_reference_cv': 'int a;const int b=1;auto&f(int n){if(n)return a;return b;}int main(){}',
 'missing_value': 'auto f(int n){if(n)return 1;return;}int main(){}',
 'void_then_value': 'auto f(int n){if(n)return;return 1;}int main(){}',
 'braced_return': 'auto f(){return {1};}int main(){}',
 'early_recursion': 'auto f(int n){return f(n);}int main(){return f(1);}',
 'template_early_recursion': 'template<class T>auto f(T n){return f(n);}int main(){return f(1);}',
 'rvalue_reference_to_lvalue': 'int n;const auto&&f(){return n;}int main(){}',
 'lvalue_reference_to_prvalue': 'auto&f(){return 1;}int main(){}',
 'pointer_return_nonpointer': 'auto*f(){return 1;}int main(){}',
 'empty_reference_function': 'auto&f(){}int main(){}',
 'condition_self_initializer': 'int n=7;int main(){if(auto n=n)return 1;}',
 'lambda_conflicting_returns': 'int main(){auto f=[](int n){if(n)return 1;return 2L;};}',
 'template_address_mismatch': 'template<class T>auto f(T x){return 3;}long(*p)(long)=f;int main(){}',
 'constexpr_nonliteral_parameter': 'struct X{~X(){}};template<class T>constexpr auto f(X x,T t){return t;}int main(){}',
 'constexpr_nonliteral_result': 'struct X{X(){};~X(){}};constexpr auto f(){return X();}int main(){}',
}

if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(), Path(sys.argv[2])) else 1)
