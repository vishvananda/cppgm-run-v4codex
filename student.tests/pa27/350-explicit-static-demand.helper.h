template<class T> struct Holder { static int value; };
template<class T> int Holder<T>::value = 19;
