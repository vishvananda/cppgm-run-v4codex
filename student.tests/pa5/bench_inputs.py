#!/usr/bin/env python3
"""Deterministic parser workloads. No timing or expected outputs in the compiler."""
import pathlib
import sys


def workloads(scale=1):
    n = 12000 * scale
    declarations = ''.join(f'int variable_{i}={i}+2*3;\n' for i in range(n))
    expressions = 'int compute(int x,int y){\n' + ''.join(
        'x=(x+y)*3; if(x>7) y=x-y; else y=y+1;\n' for _ in range(n)) + 'return x+y;}\n'
    templates = ''.join(
        f'template<class T,int N> struct box_{i}{{T data[N]; T get(int i){{return data[i];}}}};\n'
        f'box_{i}<int,4> object_{i};\n' for i in range(2500*scale))
    depth = 128 * scale
    nested = 'template<class T> struct box {};\n' + ''.join(
        'box<'*depth + 'int' + '>'*depth + f' value_{i};\n' for i in range(80))
    # Complete-class category lookahead must skip already-indexed nested bodies.
    classes = ''.join('struct C%d {' % i for i in range(depth)) + 'int value;' + '};'*depth
    classes = classes + '\n' + ''.join(f'C0 root_{i};\n' for i in range(12000))
    procedural = ''.join(
        f'double kernel_{i}(double* data,int n){{double sum=0.0;'
        'for(int j=0;j<n;++j){sum=sum+data[j]*0.5;}return sum;}'
        f'double caller_{i}(double* p,int n){{return kernel_{i}(p,n);}}\n'
        for i in range(2500*scale))
    return {'procedural': procedural, 'declarations': declarations, 'expressions': expressions,
            'templates': templates, 'nested': nested, 'classes': classes}


if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1]); root.mkdir(parents=True, exist_ok=True)
    for scale in (1, 4):
        for name, source in workloads(scale).items():
            (root / f'{name}-{scale}.cpp').write_text(source)
