#!/usr/bin/env python3
"""Fixed PA6 workloads; later executable benchmarks remain outside this stage."""
import importlib.util
import pathlib

def workloads():
    root=pathlib.Path(__file__).resolve().parents[2]
    spec=importlib.util.spec_from_file_location('pa5_inputs',root/'student.tests/pa5/bench_inputs.py')
    pa5=importlib.util.module_from_spec(spec);spec.loader.exec_module(pa5)
    result={}
    for scale in (1,4):
        for name,src in pa5.workloads(scale).items():
            result[f'ast-{name}-{scale}']=dict(source=src,mode='--emit-ast',repeats=8 if name=='nested' else 4,scale=scale,group=name)
        n=6000*scale
        declarations=''.join(f'const int bound_{i}=3;int array_{i}[bound_{i}];\n' for i in range(n))
        templates=''.join(f'template<class T>struct Box_{i}{{T member;T get(T input){{T local;return input;}}}};\n' for i in range(2000*scale))
        namespaces=''.join(f'namespace N_{i}{{using T=int;namespace A{{using V=T;}}}}namespace M_{i}{{using namespace N_{i}::A;V object;}}\n' for i in range(2500*scale))
        signatures='using F=int(const int);using P0=F*;\n'+''.join(f'using P{i}=P{i-1}*;\n' for i in range(1,65))
        signatures+=''.join(f'void f_{i}(P64);\n' for i in range(n))
        for name,src in [('constants',declarations),('templates',templates),('namespaces',namespaces),('signatures',signatures)]:
            result[f'types-{name}-{scale}']=dict(source=src,mode='--emit-types',repeats=4,scale=scale,group=name)
    return result
