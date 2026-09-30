template<int N> struct alignas(64) Storage { long values[N]; };
int main(int argc, char**) {
    Storage<8> storage;
    long dynamic[8];
    storage.values[argc & 7] = 11;
    dynamic[0] = storage.values[argc & 7] + 31;
    return dynamic[0] != 42 || (reinterpret_cast<unsigned long>(&storage) & 63) != 0;
}
