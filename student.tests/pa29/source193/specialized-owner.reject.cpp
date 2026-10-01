template<class T> struct Box { struct Inner { static int value(); }; };
template<> struct Box<int> { struct Inner { static int value(); }; };
template<> int Box<int>::Inner::value(){return 2;}
