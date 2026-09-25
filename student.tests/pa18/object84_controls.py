#!/usr/bin/env python3
"""Constructor entry, empty aggregate and observable lifetime checks: CC WORK."""
from pathlib import Path
import json,re,sys
import ordering_controls as runner
runner.GOOD={
 'empty_local':'int main(){struct E{};E a{},b{};return &a==&b;}',
 'empty_temporary':'struct E{};int calls;int f(E){return ++calls;}int main(){return f(E{})!=1||f(E{})!=2;}',
 'empty_local_template':'template<class T>int f(T){return sizeof(T);}int main(){struct E{};return f(E{})!=1;}',
 'empty_destructor':'int n;struct E{~E(){++n;}};int f(E){return n;}int main(){{E e{};if(f(E{})!=0)return 1;if(n!=1)return 2;}return n!=2;}',
 'empty_nested':'struct E{};struct A{E e;int n;};int main(){A a{{},3};return a.n!=3;}',
 'empty_array':'struct E{};int main(){E a[3]={{},{},{}};return &a[0]==&a[1]||sizeof(a)!=3;}',
 'empty_return':'struct E{};E make(){return E{};}int main(){E a=make(),b=make();return &a==&b;}',
 'local_member_template':'struct C{template<class T>int f(T){return sizeof(T);}};int main(){struct E{};C c;return c.f(E{})!=1;}',
 'local_constructor_template':'struct C{int n;template<class T>C(T):n(sizeof(T)){}};int main(){struct E{};C c(E{});return c.n!=1;}',
 'nonempty_helper':'struct A{int x,y;};int main(){A a{3,5};return a.x+a.y!=8;}',
 'aggregate_effects':'int n;int next(){return ++n;}struct A{int x,y;};int main(){A a{next(),next()};return a.x!=1||a.y!=2||n!=2;}',
 'delegate_base':'struct B{int n;B():n(3){}B(int):B(){}};struct D:B{D():B(1){}};int main(){D d;return d.n!=3;}',
 'delegate_complete':'struct B{int n;B():n(3){}B(int):B(){}};int main(){B b(1);return b.n!=3;}',
 'delegate_both_late_base':'struct B{int n;B():n(3){}B(int):B(){}};int first(){B b(1);return b.n;}struct D:B{D():B(1){}};int main(){D d;return first()!=3||d.n!=3;}',
 'delegate_both_late_complete':'struct B{int n;B():n(3){}B(int):B(){}};struct D:B{D():B(1){}};int first(){D d;return d.n;}int main(){B b(1);return first()!=3||b.n!=3;}',
 'delegate_chain':'struct B{int n;B():n(3){}B(int):B(){}B(int,int):B(1){}};struct D:B{D():B(1,2){}};int main(){D d;B b(1,2);return b.n!=3||d.n!=3;}',
 'delegate_template':'template<class T>struct B{int n;B():n(3){}template<class U>B(U):B(){}};struct D:B<int>{using B<int>::B;};int main(){D d(1);return d.n!=3;}',
 'delegate_template_both':'template<class T>struct B{int n;B():n(3){}template<class U>B(U):B(){}};struct D:B<int>{using B<int>::B;};int main(){D d(1);B<int> b(1);return d.n!=3||b.n!=3;}',
 'delegate_effects':'int n;struct B{B():x(++n){}B(int):B(){++n;}int x;};struct D:B{D():B(1){++n;}};int main(){D d;return d.x!=1||n!=3;}',
 'delegate_destructor':'int n;struct B{B(){}B(int):B(){}~B(){++n;}};struct D:B{D():B(1){}};int main(){{D d;}return n!=1;}',
 'delegate_convergent':'struct B{int n;B():n(7){}B(int):B(){}B(double):B(){}};struct D:B{D():B(1){}};struct E:B{E():B(1.0){}};int main(){D d;E e;B b(1);return d.n!=7||e.n!=7||b.n!=7;}',
}
runner.BAD={
 'delegate_cycle':'struct B{B():B(1){}B(int):B(){}};int main(){B b;}',
 'delegate_private':'class B{B(int){}public:B():B(1){}};int main(){B b(1);}',
 'delegate_multiple':'struct B{int n;B():B(1),n(2){}B(int):n(3){}};int main(){B b;}',
 'empty_excess':'struct E{};int main(){E e{1};}',
}
runner.GOOD.update({
 'empty_value_reference':'struct E{};int read(const E&e){return *reinterpret_cast<const unsigned char*>(&e);}int main(){return read(E());}',
 'empty_value_effects':'int n;struct E{};E f(){++n;return E();}int take(E){return n;}int main(){return take(f())!=1;}',
 'empty_value_defaulted_copy':'struct E{E()=default;E(const E&)=default;};int take(E){return 0;}int main(){E e{};return take(e);}',
 'empty_value_const':'struct E{};int take(E){return 0;}int main(){const E e{};return take(e);}',
 'empty_value_destructor_lvalue':'int n;struct E{~E(){++n;}};int take(E){return n;}int main(){{E e{};if(take(e)!=0||n!=1)return 1;}return n!=2;}',
})
runner.BAD.update({
 'empty_deleted_copy':'struct E{E()=default;E(const E&)=delete;};int take(E){return 0;}int main(){E e{};return take(e);}',
 'empty_private_copy':'struct E{E()=default;private:E(const E&)=default;};int take(E){return 0;}int main(){E e{};return take(e);}',
 'empty_move_assignment_deletes_copy':'struct E{E&operator=(E&&)=default;};int take(E){return 0;}int main(){E e{};return take(e);}',
 'empty_volatile_copy':'struct E{};int take(E){return 0;}int main(){volatile E e{};return take(e);}',
})

runner.GOOD.update({
 'empty_value_parameter':'namespace n{struct E{};template<class T>T apply(E,T v){return v;}}int main(){return apply(n::E(),3)!=3;}',
 'empty_value_placement':'void*operator new(unsigned long,void*p){return p;}struct E{};int main(){unsigned char a[sizeof(E)]={127};E*p=new(a)E();return a[0]!=0;}',
 'widen_signed_multiply':'long f(long n){return n*3;}int main(){return f(-7)!=-21;}',
 'widen_unsigned_multiply':'unsigned long f(unsigned long n){return n*3;}int main(){return f(7)!=21;}',
 'widen_template_multiply':'template<class T>T f(T n){return n*3;}int main(){return f(-7L)!=-21;}',
 'widen_divide':'long f(long n){return n/3;}int main(){return f(-22)!=-7;}',
 'widen_mod':'long f(long n){return n%3;}int main(){return f(-22)!=-1;}',
 'widen_bits':'long f(long n){return n&255;}int main(){return f(-1)!=255;}',
 'widen_compound':'long f(long n){n*=3;return n;}int main(){return f(-7)!=-21;}',
 'widen_negative':'long f(long n){return n*(-3);}int main(){return f(7)!=-21;}',
 'widen_offsets':'long f(long n){return n+3;}int main(){return f(-7)!=-4;}',
})

runner.GOOD.update({
 'delegate_polymorphic_base':'struct B{int n;B():n(3){}B(int):B(){}virtual int f(){return n;}};struct D:B{D():B(1){}};int main(){D d;B*p=&d;return p->f()!=3;}',
 'delegate_polymorphic_both':'struct B{int n;B():n(3){}B(int):B(){}virtual int f(){return n;}};struct D:B{D():B(1){}};int main(){D d;B b(1);return b.f()!=3||d.f()!=3;}',
 'delegate_zero_base':'struct B{int n;B():B(0){}B(int x):n(x){}};struct D:B{};int main(){D d{};return d.n;}',
 'empty_explicit_copy':'struct E{E()=default;explicit E(const E&)=default;};int main(){E e{};E f(e);}',
})
runner.BAD.update({
 'empty_explicit_copy_argument':'struct E{E()=default;explicit E(const E&)=default;};void take(E){}int main(){E e{};take(e);}',
 'empty_private_destructor':'class E{~E(){}};int take(E){return 0;}int main(){return take(E{});}',
})

if __name__=='__main__':
 cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);ok=runner.run(cc,work)
 rows=[]
 for name in ('empty_local','empty_temporary','empty_local_template','empty_destructor','empty_return'):
  path=work/(name+'.lowir');ir=path.read_text() if path.exists() else '';rows.append(dict(name=name,passed=bool(ir) and '@__aggregate_' not in ir))
 for name in ('delegate_base','delegate_template','delegate_both_late_base','delegate_both_late_complete','delegate_template_both','delegate_chain','delegate_convergent'):
  ir=(work/(name+'.lowir')).read_text();base=re.search(r'function (@[^\s(]+)\([^\n]*object=_ZN1B(?:IiE)?C[12]Ev[^\n]*object_root=yes',ir)
  rows.append(dict(name=name,passed=bool(base) and ('call void '+base[1]+'(' in ir)))
 for name in ('empty_value_parameter','empty_value_reference','empty_value_placement'):
  path=work/(name+'.lowir');ir=path.read_text() if path.exists() else ''
  rows.append(dict(name=name,passed=len(re.findall(r'^    zeroinit 1x1 ',ir,re.M))==1))
 for name in ('widen_signed_multiply','widen_template_multiply','widen_divide','widen_mod','widen_bits','widen_compound'):
  path=work/(name+'.lowir');ir=path.read_text() if path.exists() else ''
  rows.append(dict(name=name,passed='convert sext i64 i32' in ir))
 (work/'inspection.json').write_text(json.dumps(rows,indent=2)+'\n')
 for row in rows:print(row)
 sys.exit(0 if ok and all(r['passed'] for r in rows) else 1)
