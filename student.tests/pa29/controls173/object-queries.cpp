template<class T>struct C{
 T n;T run(){
   auto f=[this]<class U>(U x) noexcept(noexcept(this->n+x)) -> decltype(this->n+x) {return n+x;};
   static_assert(noexcept(f(2)),"noexcept member");
   return f(2);
 }
};
int main(){C<int> c={3};return c.run()==5?0:1;}
