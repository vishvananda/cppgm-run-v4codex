int copies;
struct Base {long first,second;Base(long a,long b):first(a),second(b){} Base(Base const&)=default;};
struct Pair:Base {Pair(long a,long b):Base(a,b){} Pair(Pair const& x):Base(x){}};
long take(Pair value){Pair local(value);return local.first+local.second;}
struct Effect:Base {Effect(long a):Base(a,a+1){} Effect(Effect const& x):Base(x){++copies;}};
long observe(Effect value){return value.first+value.second;}
struct Identity:Base {
 const Identity* self;
 Identity(int n):Base(n,n+1),self(this){}
 Identity(Identity const& x):Base(x),self(this){}
};
long identity(Identity value){if(value.self!=&value)__builtin_abort();return value.first+value.second;}
struct Other:Base {Other(int n):Base(n,n+1){} Other(Other const&):Base(8,9){}};
long other(Other value){return value.first+value.second;}
struct Escaped;
const Escaped* copied_address;
struct Escaped:Base {
 Escaped(int n):Base(n,n+1){}
 Escaped(Escaped const& x):Base((copied_address=this,x)){}
};
long escaped(Escaped value){if(copied_address!=&value)__builtin_abort();return value.first+value.second;}
void* pointer(){return reinterpret_cast<void*>(16);}
int main(){
 for(int i=0;i<50;++i){
  Pair pair(i,i+1);if(take(pair)!=2*i+1)return 1;
  Effect effect(i);int prior=copies;if(observe(effect)!=2*i+1||copies!=prior+1)return 2;
  Identity object(i);if(identity(object)!=2*i+1||object.self!=&object)return 3;
  Other changed(i);if(other(changed)!=17)return 4;
  Escaped source(i);if(escaped(source)!=2*i+1)return 6;
 }
 if(pointer()!=reinterpret_cast<void*>(16))return 5;
 return reinterpret_cast<unsigned long>(pointer())!=16;
}
