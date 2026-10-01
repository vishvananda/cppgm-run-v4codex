constexpr int site(int n=__builtin_LINE()) { return n; }
template<class T> struct Box {
#line 20 "inline.cpp"
  inline static int value=site();
  inline static const char* name=__builtin_FUNCTION();
};
#line 100 "caller.cpp"
int main() { return Box<int>::value==20 && Box<int>::name[0]==0 ? 0 : 1; }
