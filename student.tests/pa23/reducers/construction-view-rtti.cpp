namespace std { class type_info { public: bool operator==(const type_info&) const; }; }
int bad;
struct V { virtual void f() {} };
struct A : virtual V { A(); };
A::A() {
  V& v = *this;
  if (!(typeid(v) == typeid(A))) ++bad;
  if (dynamic_cast<void*>(&v) != static_cast<void*>(this)) ++bad;
}
struct P { virtual void p() {} };
struct D : P, A {};
int main() { D d; return bad; }
