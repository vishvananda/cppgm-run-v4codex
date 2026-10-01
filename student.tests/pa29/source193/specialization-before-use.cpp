template<class T> struct Box { struct Inner { static int value(){return 1;} }; };
Box<int>::Inner object;
using Signature=decltype(Box<int>::Inner::value());
template<> int Box<int>::Inner::value(){return 2;}
int main(){return Box<int>::Inner::value()!=2;}
