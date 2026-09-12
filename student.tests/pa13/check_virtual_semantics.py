#!/usr/bin/env python3
"""Independent declaration/covariance controls; run explicitly from the root."""
import pathlib, subprocess, tempfile, sys
compiler = sys.argv[1] if len(sys.argv)>1 else "dev/cppgm++"
cases = [
 ('ref-overload', 'struct B { virtual int f() &; virtual int f() &&; }; struct D:B { int f() & override; int f() && override; };', True),
 ('ref-mismatch', 'struct B { virtual int f() &; }; struct D:B { int f() && override; };', False),
 ('cv-overload', 'struct B { virtual int f(); virtual int f() const; }; struct D:B { int f() const override; };', True),
 ('private-covariance-self', 'struct B { virtual B* f(); }; struct D:private B { D* f() override; };', True),
 ('unrelated-covariance', 'struct X {}; struct B { virtual B* f(); }; struct D:B { X* f() override; };', False),
 ('reduced-return-cv', 'struct B { virtual const B& f(); }; struct D:B { D& f() override; };', True),
 ('pure-nonvirtual', 'struct B { void f() = 0; };', False),
 ('static-virtual', 'struct B { static virtual void f(); };', False),
 ('final-nonvirtual', 'struct B { void f() final; };', False),
 ('inherited-final', 'struct B { virtual void f() final; }; struct D:B {}; struct E:D { void f(); };', False),
 ('destructor-override', 'struct B { virtual ~B(); }; struct D:B { ~D() override; };', True),
 ('noexcept-override', 'struct B{virtual void f() noexcept;};struct D:B{void f() override;};', False),
 ('throwing-member-destructor', 'struct M{~M()noexcept(false);};struct B{M m;virtual ~B(){} };struct D:B{~D()noexcept(false)override{}};', True),
 ('throwing-override-destructor', 'struct M{~M()noexcept(false);};struct B{virtual ~B(){} };struct D:B{M m;};', False),
 ('destructor-final', 'struct B { virtual ~B() final; }; struct D:B {};', False),
]
with tempfile.TemporaryDirectory(prefix='pa13-semantic-') as work:
 for name, source, valid in cases:
  src=pathlib.Path(work)/(name+'.cpp'); src.write_text(source+'\nint main() { return 0; }\n')
  result=subprocess.run([compiler,'--emit-lowir','-O0','-o',str(pathlib.Path(work)/'out'),str(src)],capture_output=True)
  assert (result.returncode == 0) == valid, (name,result.stderr.decode())
print(f'{len(cases)} virtual semantic controls passed')
