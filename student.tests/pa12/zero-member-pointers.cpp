struct Value { int a; int get() { return a; } };
typedef int Value::*Data;
typedef int (Value::*Method)();
struct Pointers { Data data; Method method; };
struct Many { Pointers p[3][5]; };
struct Owner { Many many; Owner() : many() {} };
int bytes(void const* value, unsigned size, unsigned expected) {
  unsigned char const* p = reinterpret_cast<unsigned char const*>(value);
  for (unsigned i=0; i<size; ++i) if (p[i]!=expected) return 1;
  return 0;
}
int main() {
  Data data{}; Method method{}; Data array[19]{};
  Pointers p=Pointers(); Owner owner;
  if (bytes(&data,sizeof(data),255) || bytes(&method,sizeof(method),0)) return 1;
  if (bytes(&array,sizeof(array),255)) return 2;
  if (bytes(&p.data,sizeof(p.data),255) || bytes(&p.method,sizeof(p.method),0)) return 3;
  for (int i=0; i<3; ++i) for (int j=0; j<5; ++j) {
    Pointers& at=owner.many.p[i][j];
    if (bytes(&at.data,sizeof(at.data),255) || bytes(&at.method,sizeof(at.method),0)) return 4;
  }
}
