struct Point {int field;};
template<class T>auto value(T a)->decltype(a){return a;}
template<class T>auto reference(T& a)->decltype((a)){return a;}
template<class T>auto field(T a)->decltype(a.field){return a.field;}
int instantiate(){int i=3;Point p={4};return value(i)+reference(i)+field(p);}
