template<class T> struct Box { struct Inner { static int value(){return 1;} }; };
int use(){return Box<int>::Inner::value();}
template<> int Box<int>::Inner::value(){return 2;}
