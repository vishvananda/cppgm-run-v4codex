template<class T> int value(T x){return x+1;}
int use(){return value(1);}
template<> int value<int>(int x){return x+2;}
