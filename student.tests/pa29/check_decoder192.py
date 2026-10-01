#!/usr/bin/env python3
"""Independent rational rounding oracle for the bounded literal decoder."""
import fractions,json,pathlib,random,subprocess,sys
sys.set_int_max_str_digits(40000)
rng=random.Random(192)
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
exe=out/'decoder'
subprocess.run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),str(root/'student.tests/pa29/source192/decoder.cpp'),str(root/'dev/src/support/extended_float.cpp'),'-o',exe],check=True)
F=fractions.Fraction
def number(s):
 neg=s.startswith('-');s=s.lstrip('+-')
 if s.startswith('0x'):
  a,b=s[2:].split('p');parts=a.split('.');places=len(parts[1]) if len(parts)>1 else 0
  n=F(int(''.join(parts),16))*F(2)**(int(b)-4*places)
 else:n=F(s)
 return -n if neg else n

def expected(s,p):
 sign=int(s.startswith('-'));n=abs(number(s));eb=5 if p==11 else 15;bias=2**(eb-1)-1
 if not n:return sign<<(p+eb-1)
 e=n.numerator.bit_length()-n.denominator.bit_length()
 if n<F(2)**e:e-=1
 unit=max(e,1-bias)-(p-1);scaled=n/F(2)**unit;q,r=divmod(scaled.numerator,scaled.denominator)
 if r*2>scaled.denominator or (r*2==scaled.denominator and q&1):q+=1
 if q>=2**p:q>>=1;e+=1
 if e>bias:return (sign<<(p+eb-1))|((2**eb-1)<<(p-1))
 if q<2**(p-1):b=q
 else:b=((max(e,1-bias)+bias)<<(p-1))|(q-2**(p-1))
 return (sign<<(p+eb-1))|b
cases=[]
for p in [11,113]:
 for s in ['0','-0','1','-1','1.00048828125','1.00146484375','0x1p-25','0x1.8p-25','0x1p-24','0x1p-14','65519','65520','0x1p-16494','0x1p-16495','0x1.8p-16494','0x1.ffffffffffffffffffffffffffffp16383','0x1.ffffffffffffffffffffffffffff8p16383']:
  cases.append((p,s))
 for i in range(800):
  digits=''.join(str(rng.randrange(10)) for _ in range(rng.randrange(1,150)))
  exponent=rng.randrange(-60,40) if p==11 else rng.randrange(-5150,4950)
  cases.append((p,('-' if rng.randrange(2) else '')+digits[0]+'.'+digits[1:]+'e'+str(exponent)))
 for sign in ['', '-']:
  for s in ['1.00048828125'+'0'*20000+'1','1.00000000000000000000000000000000009629649721936179265279889712924636592690508241076940976199693977832794189453125'+'0'*20000+'1']:
   cases.append((p,sign+s))
text=''.join(f'{p} {s}\n' for p,s in cases)
result=subprocess.run([exe],input=text,text=True,capture_output=True,check=True)
actual=result.stdout.splitlines();assert len(actual)==len(cases)
fail=[]
for (p,s),got in zip(cases,actual):
 want=expected(s,p)
 if int(got,16)!=want:fail.append(dict(precision=p,input=s,actual=got,expected=hex(want)))
record=dict(cases=len(cases),failed=len(fail),failures=fail)
(out/'decoder.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps({k:v for k,v in record.items() if k!='failures'}))
assert not fail,fail[:5]
