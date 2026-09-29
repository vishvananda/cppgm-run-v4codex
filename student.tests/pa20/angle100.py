#!/usr/bin/env python3
"""Retained angle grammar: point of declaration, ADL, scopes and source reuse."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

HEAD = '''int hits;
template<int = 0> struct box {
  static const int member = 0;
  friend void operator>(int, const box&) { ++hits; }
};
struct large { box<> data[2]; };
template<int> struct select;
'''
VALUE = 'template<>struct select<sizeof(box<>)>{static const int pick=0;};\n'
TYPE = 'template<>struct select<sizeof(large)>{template<int>struct pick{static const int member=0;};};\n'
DECL = 'box<select<sizeof(value)>::pick<0>::member> value;'
BODY = DECL + '{' + DECL + '}'
PREFIX = HEAD + VALUE + TYPE + 'extern large value;\n'
runner.GOOD = {
    'required': PREFIX + 'int main(){' + BODY + 'return hits!=1;}',
    'reverse_specializations': HEAD + TYPE + VALUE + 'extern large value;int main(){' + BODY + 'return hits!=1;}',
    'siblings_restore_scope': PREFIX + 'int main(){ {' + BODY + '} {' + BODY + '} return hits!=2;}',
    'repeat_expression': PREFIX + 'int main(){' + DECL + '{' + DECL*3 + '}return hits!=3;}',
    'three_blocks': PREFIX + 'int main(){' + DECL + '{{{' + DECL + '}}}return hits!=1;}',
    'for_body': PREFIX + 'int main(){' + DECL + 'for(int i=0;i<3;++i){' + DECL + '}return hits!=3;}',
    'for_init': PREFIX + 'int main(){' + DECL + 'for(' + DECL + 'false;){}return hits!=1;}',
    'if_branch': PREFIX + 'int main(){' + DECL + 'if(true){' + DECL + '}else{' + DECL + '}return hits!=1;}',
    'unbraced_if': PREFIX + 'int main(){' + DECL + 'if(true)' + DECL + 'return hits!=1;}',
    'namespace': 'namespace n{' + PREFIX + 'int f(){' + BODY + 'return hits!=1;}}int main(){return n::f();}',
    'ordinary_function': PREFIX + 'int f(){' + BODY + 'return hits!=1;}int main(){return f();}',
    'parameter': PREFIX + 'int f(box<>value){' + DECL + 'return hits!=1;}int main(){return f(box<>());}',
    'member': PREFIX + 'struct S{int f(){' + BODY + 'return hits!=1;}};int main(){S s;return s.f();}',
    'out_of_line_member': PREFIX + 'struct S{int f();};int S::f(){' + BODY + 'return hits!=1;}int main(){S s;return s.f();}',
    'lambda': PREFIX + 'int main(){auto f=[](){' + BODY + '};f();return hits!=1;}',
    'function_template': PREFIX + 'template<class T>void f(){' + BODY + '}int main(){f<int>();f<long>();return hits!=2;}',
    'class_template': PREFIX + 'template<class T>struct S{void f(){' + BODY + '}};int main(){S<int>s;S<long>t;s.f();t.f();return hits!=2;}',
    'template_lambda': PREFIX + 'template<class T>void f(){auto g=[](){' + BODY + '};g();}int main(){f<int>();f<long>();return hits!=2;}',
    'alias_qualifier': PREFIX + 'template<int N>using alias=select<N>;int main(){' + BODY.replace('select<','alias<') + 'return hits!=1;}',
    'renamed': (PREFIX + 'int main(){' + BODY + 'return hits!=1;}').replace('select','route').replace('pick','kind').replace('box','record').replace('member','result').replace('value','arg'),
    'relational_argument': PREFIX + 'int main(){' + BODY.replace('pick<0>','pick<(1-1)>') + 'return hits!=1;}',
}
runner.BAD = {
    'explicit_template_is_not_relational': PREFIX + 'int main(){' + DECL + '{' + DECL.replace('::pick','::template pick') + '}}',
    'missing_rhs': PREFIX + 'int main(){box<>value;' + DECL.replace('> value;', '> absent;') + '}',
}
runner.GOOD.update({
    'qualified_relation': PREFIX + 'int main(){box<>value;select<sizeof(value)>::pick<0> value;return hits!=1;}',
    'unclosed_relation': PREFIX + 'int main(){return select<sizeof(box<>)>::pick < 1 ? 0 : 1;}',
    'unclosed_statement': PREFIX + 'int main(){select<sizeof(box<>)>::pick < 1;}',
    'dereferenced_operand': PREFIX + 'int main(){' + DECL + 'box<>*p=&value;{' + DECL.replace('> value;','> *p;') + '}return hits!=1;}',
    'parenthesized_operand': PREFIX + 'int main(){' + DECL + '{' + DECL.replace('> value;','> (value);') + '}return hits!=1;}',
    'subscript_operand': PREFIX + 'int main(){' + DECL + 'box<>a[2];{' + DECL.replace('> value;','> a[1];') + '}return hits!=1;}',
    'call_operand': PREFIX + 'box<> object;box<>&get(){return object;}int main(){' + DECL + '{' + DECL.replace('> value;','> get();') + '}return hits!=1;}',
    'multiple_arguments': (PREFIX + 'int main(){' + BODY.replace('pick<0>','pick<0,1>') + 'return hits!=1;}').replace('template<int = 0>','template<int = 0,int = 0>').replace('template<int>struct pick','template<int,int>struct pick'),
})
for name, operand, value, expected in (
    ('logical_or','0||1',2,2), ('logical_and','0&&0',-1,1),
    ('bitwise_or','0|1',2,2), ('equality','0==0',2,2),
    ('conditional','0?0:1',2,2), ('relational','0<1',2,2),
    ('arithmetic','1+2',2,2), ('parenthesized','(0||1)',2,1),
):
    head = PREFIX.replace('template<int = 0>','template<int N = 0>').replace('member = 0','member = N').replace('int, const box&','int x, const box&').replace('++hits','hits+=x+1').replace('int pick=0','int pick='+str(value))
    runner.GOOD['precedence_'+name] = head+'int main(){'+DECL+'{'+DECL.replace('pick<0>','pick<'+operand+'>')+'}return hits!='+str(expected)+';}'
for n in (40,600):
    runner.GOOD['unclosed_chain_'+str(n)] = PREFIX+'int main(){return !('+' && '.join('select<sizeof(box<>)>::pick < 1' for _ in range(n))+');}'
if __name__ == '__main__':
    sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
