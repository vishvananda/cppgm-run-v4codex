int main() {
    volatile int data[10]={0,1,2,3,4,5,6,7,8,9};
    auto [a,b,c,d,e,f,g,h,i,j]=data;
    static_assert(__is_same(decltype(a),volatile int),"volatile copy element");
    return a+b+c+d+e+f+g+h+i+j!=45;
}
