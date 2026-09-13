struct Large {long words[4];};
struct Fixed {int n;Fixed(int n):n(n){}int get()const{return n;}};
struct Base {protected:int base;Base():base(4){}};
template<class T>struct Box:Base {
    T padding;
    int n;
    mutable int edits;
    int& ref;
    Fixed data;
    unsigned small:3;
    unsigned wide:32;
    Box(int& r):n(3),edits(0),ref(r),data(7),small(5),wide(0xffffffffu){}
    int run(int k){n+=k;++this->n;(*this).small^=1;++data.n;return data.get()+n+ref+base+small;}
    int read()const{return this->n+ref+base+ ++edits;}
    int& reference()const{return this->ref;}
    int local(){int n=100;return n+this->n;}
    static unsigned width(){return sizeof(n);}
    bool promote(){return small+0==5 && wide+0>0;}
    int unused(){return T::missing;}
    struct Nested {int n;int get()const{return n+1;}};
};
template<class T>struct Width {
    unsigned bits:sizeof(T)*8;
    bool check(){bits=0xffffffffu;return bits+0>0;}
};
template<class T>int exercise(){
    int r=2;Box<T> a(r);const Box<T>& c=a;
    typename Box<T>::Nested n;n.n=8;
    if(!a.promote() || a.run(2)!=24 || c.read()!=13 || c.read()!=14)return 1;
    c.reference()=9;
    if(r!=9 || a.local()!=106 || Box<T>::width()!=sizeof(int) || n.get()!=9)return 2;
    return 0;
}
template<class T>int forward(){
    struct Local {int n;int get(){return n+1;}};
    struct Local;
    Local value;value.n=6;return value.get();
}
template<class T>struct Out {
    T padding; int n;
    Out();int get()const;int get(int);static int get(long);int& change();
    static int pair(int k,decltype(k) l);int pair(long,long);
    static unsigned width();
    struct Nested {int n;int get()const;};
};
template<class U>Out<U>::Out():n(8){}
template<class U>int Out<U>::get()const{return this->n;}
template<class U>int Out<U>::get(int k){return n+k;}
template<class U>int Out<U>::get(long k){return k+1;}
template<class U>int& Out<U>::change(){return ++(n);}
template<class U>int Out<U>::pair(int k,decltype(k) l){return k+l;}
template<class U>int Out<U>::pair(long k,long l){return n+k+l;}
template<class U>unsigned Out<U>::width(){return sizeof(n);}
template<class U>int Out<U>::Nested::get()const{return n+2;}
template<class T>int out(){
    Out<T> o;typename Out<T>::Nested n;n.n=3;
    if(o.get()!=8 || o.get(3)!=11 || Out<T>::get(2L)!=3 || Out<T>::width()!=sizeof(int) || n.get()!=5)return 1;
    o.change()=10;return o.get()!=10 || Out<T>::pair(2,3)!=5 || o.pair(2L,3L)!=15;
}
int main(){Width<char> narrow;Width<unsigned> wide;return exercise<int>() || exercise<Large>() || !narrow.check() || !wide.check() || forward<int>()!=7 || forward<Large>()!=7 || out<int>() || out<Large>();}
