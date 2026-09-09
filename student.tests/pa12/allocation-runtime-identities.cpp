typedef decltype(sizeof(0)) size_t;
int main() {
    void *(*scalar)(size_t) = &::operator new;
    void *(*array)(size_t) = &::operator new[];
    void (*free_scalar)(void *) = &::operator delete;
    void (*free_array)(void *) = &::operator delete[];
    if (scalar == array || free_scalar == free_array) return 1;
    int *a = static_cast<int*>(scalar(sizeof(int)));
    int *b = static_cast<int*>(array(3*sizeof(int)));
    *a = 7; b[2] = 9;
    int result = *a + b[2];
    free_scalar(a); free_array(b);
    return result != 16;
}
