#!/usr/bin/env python3
"""Independent full-stage PA7 probes; no course oracle or host compiler."""
import pathlib
import subprocess
import sys
import tempfile

compiler = str(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve())
# Expected types and validity follow the PA7 slice and N3485's expression rules.
cases = [
    ('returned pointer call', 'int (*choose())(int);int f(){return choose()(1);}', True,
     'call-expression prvalue pointer to function of (int) returning int'),
    ('returned reference call', 'int (&choose())(int);int f(){return choose()(1);}', True,
     'call-expression lvalue lvalue-reference to function of (int) returning int'),
    ('conditional function callee', 'int f();int g();int h(bool c){return (c?f:g)();}', True,
     'conditional-expression lvalue function of () returning int'),
    ('conditional call result', 'int f();int g();int h(bool c){return c?f():g();}', True,
     'conditional-expression prvalue int'),
    ('comma call callee', 'void side();int f();int h(){return (side(),f)();}', True,
     'binary-expression lvalue function of () returning int OP_COMMA:,'),
    ('comma cast form', 'int f(){return (0,static_cast<int>(1));}', True,
     'binary-expression prvalue int OP_COMMA:,'),
    ('comma query form', 'int f(){return (0,__builtin_constant_p(1));}', True,
     'binary-expression prvalue int OP_COMMA:,'),
    ('comma abort form', 'void f(){return (0,__builtin_abort());}', True,
     'binary-expression prvalue void OP_COMMA:,'),
    ('discarded overload in comma', 'void f(int);void f(long);int h(){return (f,1);}', False, ''),
    ('indirect unresolved address', 'void f(int);void f(long);void h(){(&f)(1);}', False, ''),
    ('deep qualification ranking', 'int f(int*const*);long f(const int*const*);int h(int**p){return f(p);}', True,
     'callee f function of (pointer to const pointer to int) returning int'),
    ('deep incomparable qualification', 'void f(const int*const*);void f(volatile int*const*);void h(int**p){f(p);}', False, ''),
    ('unevaluated member body', 'struct C{void bad(){missing();}};int h(){return sizeof(&C::bad);}', True,
     'sizeof-expression prvalue unsigned long int'),
    ('rank not width', 'void pick(unsigned long);void pick(unsigned long long);void f(unsigned long a,long long b){pick(a+b);}', True,
     'callee pick function of (unsigned long long int) returning void'),
    ('signed rank', 'void pick(long);void pick(long long);void f(long a,long long b){pick(a+b);}', True,
     'callee pick function of (long long int) returning void'),
    ('unsigned rank', 'void pick(unsigned long);void pick(unsigned long long);void f(unsigned long a,unsigned long long b){pick(a+b);}', True,
     'callee pick function of (unsigned long long int) returning void'),
    ('wide signed representation', 'void pick(long);void pick(unsigned);void f(unsigned a,long b){pick(a+b);}', True,
     'callee pick function of (long int) returning void'),
    ('char32 promotion', 'void pick(unsigned);void pick(long);void f(char32_t x){pick(x);}', True,
     'callee pick function of (unsigned int) returning void'),
    ('wchar promotion', 'void pick(int);void pick(long);void f(wchar_t x){pick(x);}', True,
     'callee pick function of (int) returning void'),
    ('constant conditional conversion', 'static_assert((true?-1:1u)>0,"unsigned");', True, 'translation-unit'),
    ('constant rank conversion', 'static_assert((0UL + -1LL)>0,"unsigned");', True, 'translation-unit'),
    ('constant short circuit', 'static_assert(true || (1/0),"skip");', True, 'translation-unit'),
    ('constant dead arm', 'static_assert((true ? -1 : (1u/0))>0,"skip");', True, 'translation-unit'),
    ('constant comma', 'constexpr int x=(1,2);static_assert(x==2,"comma");', True, 'variable x const int'),
    ('volatile is not constant', 'const volatile int x=1;constexpr int y=x;', False, ''),
    ('cv pointer conditional', 'const volatile int*f(bool c,const int*p,volatile int*q){return c?p:q;}', True,
     'conditional-expression prvalue pointer to const volatile int'),
    ('deep cv pointer conditional', 'const int*const*f(bool c,int**p,const int**q){return c?p:q;}', True,
     'conditional-expression prvalue pointer to const pointer to const int'),
    ('cv pointer comparison', 'bool f(const int*p,volatile int*q){return p==q;}', True,
     'binary-expression prvalue bool OP_EQ:=='),
    ('invalid deep void composition', 'bool f(int**p,void**q){return p==q;}', False, ''),
    ('void pointer increment', 'void f(void*p){++p;}', False, ''),
    ('void pointer offset', 'void f(void*p){p+1;}', False, ''),
    ('function pointer offset', 'void f(void(*p)()){p+1;}', False, ''),
    ('incomplete pointer offset', 'struct C;void f(C*p){p+1;}', False, ''),
    ('function pointer difference', 'long f(void(*p)(),void(*q)()){return p-q;}', False, ''),
    ('pointer ordered zero', 'bool f(int*p){return p<0;}', False, ''),
    ('function pointer ordering', 'bool f(void(*p)(),void(*q)()){return p<q;}', False, ''),
    ('nullptr ordering', 'bool f(){return nullptr<nullptr;}', False, ''),
    ('pointer unary plus', 'int*f(int*p){return +p;}', True, 'unary-expression prvalue pointer to int'),
    ('qualified builtin', 'namespace N{}void f(){N::__builtin_abort();}', False, ''),
    ('qualified constant query', 'namespace N{}int f(){return N::__builtin_constant_p(1);}', False, ''),
    ('nullptr condition', 'void f(){if(nullptr){}while(nullptr){} }', True, 'while-statement'),
    ('nullptr direct bool', 'bool f(){return static_cast<bool>(nullptr);}', True, 'cast-expression prvalue bool'),
    ('nullptr copy bool', 'bool b=nullptr;', False, ''),
    ('nullptr negation', 'bool f(){return !nullptr;}', True, 'unary-expression prvalue bool'),
    ('unrelated static pointer cast', 'double*f(int*p){return static_cast<double*>(p);}', False, ''),
    ('static removes cv', 'int*f(const int*p){return static_cast<int*>(p);}', False, ''),
    ('const removes cv', 'int*f(const int*p){return const_cast<int*>(p);}', True, 'cast-expression prvalue pointer to int'),
    ('const unrelated types', 'double*f(int*p){return const_cast<double*>(p);}', False, ''),
    ('const scalar', 'int f(int x){return const_cast<int>(x);}', False, ''),
    ('reinterpret scalar', 'int f(double x){return reinterpret_cast<int>(x);}', False, ''),
    ('reinterpret pointer', 'double*f(int*p){return reinterpret_cast<double*>(p);}', True, 'cast-expression prvalue pointer to double'),
    ('reinterpret narrow integer', 'int f(int*p){return reinterpret_cast<int>(p);}', False, ''),
    ('reinterpret wide integer', 'long f(int*p){return reinterpret_cast<long>(p);}', True, 'cast-expression prvalue long int'),
    ('duplicate cases', 'void f(int x){switch(x){case 1:break;case 1:break;}}', False, ''),
    ('duplicate converted cases', 'void f(unsigned x){switch(x){case -1:break;case 4294967295u:break;}}', False, ''),
    ('duplicate defaults', 'void f(int x){switch(x){default:break;default:break;}}', False, ''),
    ('nested switch identities', 'void f(int x){switch(x){case 1:switch(x){case 1:break;default:break;}default:break;}}', True, 'switch-statement'),
    ('constexpr runtime initializer', 'int f(int x){constexpr int y=x;return y;}', False, ''),
    ('constexpr missing initializer', 'constexpr int x;', False, ''),
    ('parameter outer block redeclaration', 'void f(int x){int x;}', False, ''),
    ('local redeclaration', 'void f(){int x;int x;}', False, ''),
    ('condition redeclaration', 'void f(){if(int x=1){int x;}}', False, ''),
    ('nested shadowing', 'void f(int x){{int x;}if(int y=1){{int y;}}}', True, 'variable y int'),
]
failures = []
with tempfile.TemporaryDirectory(prefix='pa7-audit-') as directory:
    root = pathlib.Path(directory)
    src, out = root/'input.cc', root/'out'
    for name, source, success, fragment in cases:
        src.write_text(source)
        run = subprocess.run([compiler, '--emit-semantics', '-o', out, src], capture_output=True, text=True, timeout=15)
        if (run.returncode == 0) != success or (success and fragment not in out.read_text()):
            failures.append((name, run.returncode, run.stderr.strip()))
for failure in failures:
    print(failure)
assert not failures, f'{len(failures)}/{len(cases)} independent PA7 audit cases failed'
print(f'{len(cases)} independent PA7 audit cases passed')
