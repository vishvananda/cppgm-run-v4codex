template<int N>struct Box{}; template<int N> Box(unsigned _BitInt(N))->Box<N>; int main(){Box<7> b;}
