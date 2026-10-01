template<class T> struct Box { struct Inner { static int data; }; };
template<class T> int Box<T>::Inner::data=2;
template<> int Box<long>::Inner::data=3;
int main(){return Box<int>::Inner::data!=2 || Box<long>::Inner::data!=3;}
