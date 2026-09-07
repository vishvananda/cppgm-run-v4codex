#!/usr/bin/env python3
"""Exercise parser ownership boundaries independently of course discovery."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else 'dev/cppgm++').resolve()
cases = [
    ('namespace lib {struct record{}; int call();} void f(){lib::call();lib::record r;}',
     ['id-expression lib::call', 'decl-specifier lib::record']),
    ('template<class T> struct leaf{}; template<int n> struct bits{}; leaf<bits<(8>>1)>> x;',
     ['decl-specifier leaf<bits<(8>>1)>>']),
    ('template<class T> struct thing{}; int f(int thing){return thing<2;}',
     ['binary-expression OP_LT:<\n          id-expression thing']),
    ('struct base{typedef int value_type;}; struct derived:base{value_type get();};'
     'base::value_type derived::get(){return (value_type)3;}',
     ['cast-expression OP_LPAREN:', 'type-name value_type']),
    ('struct C{void f(){Later* p; p=0;} struct Later{};};',
     ['decl-specifier TT_IDENTIFIER:Later', 'assignment-expression OP_ASS:=']),
    ('struct C{int value;}; int C::*p; int f(){return 1;}',
     ['ptr-operator C::*']),
    ('struct C{}; int f(){return sizeof(C());}',
     ['sizeof-expression\n          call-expression']),
    ('struct tag{}; int fetch(); struct holder{}; void f(){holder h(tag(),fetch());}',
     ['paren-initializer', 'id-expression fetch']),
    ('int f(int callback(int),int named(int(value)),int abstract(int()));',
     ['identifier value', 'abstract-declarator']),
    ('template<class T> struct scope {static const int x=3;};'
     'template<bool B> struct flag{}; flag<(scope<int>::x>1)> x;',
     ['decl-specifier flag<(scope<int>::x>1)>']),
    ('int f(){int x=0; auto a=[&x](){return x;}; return a();}',
     ['lambda-introducer [&x]', 'id-expression a']),
    ('void f() throw(int); void g() noexcept(true){}',
     ['function-qualifier throw(int)', 'function-qualifier noexcept(true)']),
    ('struct C{C();operator int()const;}; C::C(){} C::operator int()const{return 1;}',
     ['special-member-definition C::C', 'special-member-definition C::operator int']),
    ('template<template<class> class F> struct A{}; template<class F> struct B{F* p;};',
     ['template-template-parameter', 'decl-specifier TT_IDENTIFIER:F']),
    ('namespace outer{inline namespace inner {struct type{};}} outer::type object;',
     ['decl-specifier outer::type']),
]
with tempfile.TemporaryDirectory(prefix='pa5-extended-') as tmp:
    root = pathlib.Path(tmp)
    src,out = root/'input.cpp',root/'output.ast'
    for index,(source,fragments) in enumerate(cases):
        src.write_text(source)
        result=subprocess.run([compiler,'--emit-ast','-o',out,src],capture_output=True,text=True)
        assert result.returncode==0,(index,result.stderr)
        text=out.read_text()
        for fragment in fragments:
            assert fragment in text,(index,fragment,text)
print(f'PA5 extended: {len(cases)} independent cases pass')
