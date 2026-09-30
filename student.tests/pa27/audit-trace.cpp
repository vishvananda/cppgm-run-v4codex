int destroyed;
struct Resource {
    int value;
    Resource(int n):value(n){}
    ~Resource(){++destroyed;}
};
template<int N> struct Value {
    long prefix;
    struct { Resource first; int second; };
    Value(int x):prefix(N),first(x),second(first.value+1){}
    int read() const {return int(prefix)+first.value+second;}
};
int main(int argc,char**) {
    int result;
    {Value<11> a(argc);result=a.read();}
    return result==14 && destroyed==1 ? 0:1;
}
