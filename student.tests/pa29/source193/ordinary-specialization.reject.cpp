struct Box { struct Inner { static int value(); }; };
template<> int Box::Inner::value(){return 2;}
