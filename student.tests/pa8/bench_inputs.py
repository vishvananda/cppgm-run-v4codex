"""Fixed LowIR compiler corpus and executable workloads (no fixture answers)."""

def compiler_workloads(base=3000):
    for group in ('calls-integers', 'memory-floating', 'cfg-phi'):
        for scale in (1,4):
            parts=[]
            for n in range(base*scale):
                if group=='calls-integers':
                    body='\n'.join(f'%v{k}=binary add i64 '+('%x' if k==0 else f'%v{k-1}')+',17' for k in range(20))
                    parts.append(f'function @f{n}(%x:i64)->i64 {{block ^entry:{body}\n%r=call i64 @identity(%v19) return i64 %r}}')
                elif group=='memory-floating':
                    body='\n'.join(f'%p{k}=index f64 %p,{k}\n%x{k}=load f64 %p{k}\n%v{k}=binary mul f64 %x{k},1.5\nstore f64 %v{k},%p{k}' for k in range(7))
                    parts.append(f'function @f{n}(%p:ptr)->f64 {{block ^entry:{body}\nreturn f64 %v6}}')
                else:
                    parts.append(f'''function @f{n}(%n:i64)->i64 {{
block ^entry:jump ^loop
block ^loop:%i=phi i64 [^entry:0,^latch:%next] %s=phi i64 [^entry:0,^latch:%sum]
%c=cmp lt i64 %i,%n branch %c,^latch,^exit
block ^latch:%next=binary add i64 %i,1 %sum=binary add i64 %s,%next jump ^loop
block ^exit:return i64 %s }}''')
            prefix='declare function @identity(%x:i64)->i64\n' if group=='calls-integers' else ''
            yield f'{group}-{scale}',dict(group=group,scale=scale,source=prefix+'\n'.join(parts)+'\n',functions=base*scale)

def runtime_source(group, iterations):
    # Inputs to each helper come from volatile loop state, with a checked
    # accumulated result. O0 retains calls, loads and arithmetic. All storage is
    # local, so the sectionless native ELF has a measurable pure text span.
    extra=''
    slots=''
    setup=''
    finish=''
    if group=='sum':
        assert iterations%1000001==0
        work='%input=binary umod i64 %i,1000001 %value=call i64 @sum_to(%input)'
        expected=(1000000*1000001*1000002//6)*(iterations//1000001)
    elif group=='swap':
        slots='slot $left:i64 slot $right:i64'
        setup='store i64 3,$left store i64 7,$right %left=addr $left %right=addr $right'
        work='''call void @swap_values(%left,%right) call void @swap_values(%left,%left)
%value=load volatile i64 $left call void @swap_values(%left,%right)'''
        expected=iterations*7
        finish='%a=load i64 $left %b=load i64 $right %bad_a=cmp ne i64 %a,3 %bad_b=cmp ne i64 %b,7 %bad_ab=binary or i64 %bad_a,%bad_b %bad_all=binary or i64 %bad,%bad_ab return i64 %bad_all'
    elif group=='call':
        extra='function @step(%x:i64)->i64 {block ^entry:%p=binary mul i64 %x,3 %r=binary add i64 %p,1 return i64 %r}'
        setup='%fp=addr @step'
        work='%input=binary umod i64 %i,1000 %value=call i64 @call_twice(%fp,%input)'
        assert iterations%1000==0
        expected=(9*999*1000//2+4*1000)*(iterations//1000)
    elif group=='floating':
        extra='function @scale(%p:ptr)->f64 {block ^entry:%x=load volatile f64 %p %y=binary mul f64 %x,1.5 %r=binary add f64 %y,2.0 store volatile f64 %r,%p return f64 %r}'
        slots='slot $cell:f64'
        setup='%cell=addr $cell'
        work='''%input=binary umod i64 %i,1000 %fp=convert sitofp f64 i64 %input store volatile f64 %fp,$cell
%result=call f64 @scale(%cell) %twice=binary mul f64 %result,2.0 %value=convert fptosi i64 f64 %twice'''
        assert iterations%1000==0
        expected=(3*999*1000//2+4*1000)*(iterations//1000)
    else:raise ValueError(group)
    return extra+f'''\nfunction @main()->i64 [role=entry] {{
slot $counter:i64 slot $acc:i64 {slots}
block ^entry:store volatile i64 0,$counter store volatile i64 0,$acc {setup} jump ^loop
block ^loop:%i=load volatile i64 $counter %more=cmp lt i64 %i,{iterations} branch %more,^body,^exit
block ^body:{work}
%acc=load volatile i64 $acc %sum=binary add i64 %acc,%value store volatile i64 %sum,$acc
%next=binary add i64 %i,1 store volatile i64 %next,$counter jump ^loop
block ^exit:%actual=load volatile i64 $acc %bad=cmp ne i64 %actual,{expected}
{finish or 'return i64 %bad'} }}\n'''
