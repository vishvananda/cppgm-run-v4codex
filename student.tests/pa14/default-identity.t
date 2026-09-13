int live=0,counter=0;
struct Value {
    int n;
    Value(int n):n(n){++live;}
    Value(const Value& v):n(v.n){++live;}
    ~Value(){--live;}
};
Value make(){return Value(++counter);}
const Value& value(const Value& v=make()){return v;}
int distinct(const Value& a,const Value& b){return a.n!=b.n && a.n+b.n==2*counter-1 && live==2;}
template<class T>int check(){return distinct(value(),value());}
struct Tag{};
int main(){if(!check<int>() || live || !check<Tag>() || live) return 1;return 0;}
