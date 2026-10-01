template<class T> struct Box { struct Inner { static int value(); }; };
template<> int Box<int>::Inner::value(long){return 2;}
