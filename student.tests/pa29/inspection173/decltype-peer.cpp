struct Point {int field;};
template<class T>auto value(T a)->decltype(a);
template<class T>auto reference(T& a)->decltype((a));
template<class T>auto field(T a)->decltype(a.field);
int main(){int i=3;Point p={4};reference(i)=6;return value(i)==6 && field(p)==4?0:1;}
