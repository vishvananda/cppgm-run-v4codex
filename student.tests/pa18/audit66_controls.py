#!/usr/bin/env python3
"""Shared query recomputation must not conceal completion from failed consumers.
N3485 [temp.deduct]/8: invalid immediate types discard candidates; after the
class is complete its sizeof expression is valid ([expr.sizeof]/1). These
probe declarations have no instantiated definitions whose meaning changes.
Run CC WORK; all accepted controls execute the student's validated LowIR.
"""
from pathlib import Path
import sys
import ordering_controls as runner

runner.GOOD = {
 'alias_other_consumer': 'struct A;template<class T>using arr=int[sizeof(T)];template<class T>arr<T>*f(int);template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};template<class T>int g(int(*)[sizeof(T)]);using Other=decltype(g<A>(0));static_assert(sizeof(*f<A>(0))==sizeof(int), "");int main(){}',
 'result_other_consumer': 'struct A;template<class T>auto f(int)->decltype(sizeof(T),char());template<class>long f(...);static_assert(sizeof(f<A>(0))==sizeof(long), "");struct A{};template<class T>auto g(int)->decltype(sizeof(T),char());using Other=decltype(g<A>(0));static_assert(sizeof(f<A>(0))==sizeof(char), "");int main(){}',
}

def pending_source(n):
 source='template<class T>using Arr=int[sizeof(T)];template<class T>Arr<T>*f(int);template<class>long f(...);template<class T>int g(int(*)[sizeof(T)]);'
 source+=''.join(f'struct C{i};using Before{i}=decltype(f<C{i}>(0));static_assert(sizeof(Before{i})==sizeof(long), "");' for i in range(n))
 source+='struct C0{};using Warm=decltype(g<C0>(0));static_assert(sizeof(*f<C0>(0))==sizeof(int), "");'
 # An unrelated failed fact stays failed; warm/reuse the successful fact twice.
 source+='static_assert(sizeof(f<C1>(0))==sizeof(long), "");static_assert(sizeof(*f<C0>(0))==sizeof(int), "");int main(){}'
 return source
for n in (32,128,512):
 runner.GOOD['pending_'+str(n)]=pending_source(n)
runner.BAD = {}

if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
