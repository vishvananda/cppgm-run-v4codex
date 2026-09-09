int copies;
struct Field {
  int value;
  Field():value(17) {}
  Field(const Field& other):value(other.value) { ++copies; }
  Field(Field&&)=delete;
};
struct Box { Field field; };
int main() {
  Box source;
  Box result(static_cast<Box&&>(source));
  return copies!=1 || result.field.value!=17;
}
