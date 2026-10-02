template<class T> T* allocate(unsigned n) { return new T[n]; }
template<class T> int (*matrix(unsigned n))[3] { return new int[n][3]; }
template<class T> auto shape(unsigned n) -> decltype(new T[n]) { return new T[n]; }
int main(int argc, char**) {
    int* p = allocate<int>(argc + 3);
    p[argc + 2] = 19;
    int (*q)[3] = matrix<int>(argc + 2);
    q[argc + 1][2] = 23;
    int* r = shape<int>(argc + 1);
    r[argc] = 7;
    int result = p[argc + 2] + q[argc + 1][2] + r[argc] - 49;
    delete[] p; delete[] q; delete[] r;
    return result;
}
