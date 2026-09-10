struct Value { int member; };
typedef int Value::*Data;
struct Pointers { Data data; };
unsigned extent() { return 19; }
int null_data(Data const& value) {
    unsigned char const* bytes = reinterpret_cast<unsigned char const*>(&value);
    for (unsigned i = 0; i < sizeof(Data); ++i) if (bytes[i] != 255) return 0;
    return 1;
}
int main() {
    Data* scalars = new Data[extent()]();
    Pointers* objects = new Pointers[extent()]();
    for (int i = 0; i < 19; ++i) {
        if (!null_data(scalars[i]) || !null_data(objects[i].data)) return 1;
    }
    delete[] objects;
    delete[] scalars;
    volatile int* observed = new volatile int[extent()]();
    for (int i = 0; i < 19; ++i) if (observed[i]) return 2;
    delete[] observed;
    return 0;
}
