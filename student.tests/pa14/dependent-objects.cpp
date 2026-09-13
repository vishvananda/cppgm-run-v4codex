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
int main(){Width<char> narrow;Width<unsigned> wide;return exercise<int>() || exercise<Large>() || !narrow.check() || !wide.check() || forward<int>()!=7 || forward<Large>()!=7;}
