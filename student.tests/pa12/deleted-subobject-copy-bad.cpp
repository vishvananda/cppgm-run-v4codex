struct Field { Field() {} Field(const Field&) = delete; };
struct Box { Field field; };
int main() { Box a; Box b(a); }
