template<class T> struct Arrays {
 inline static int data[3] = {1,2,3};
 inline static const int empty[] = {4,5};
};
static_assert(sizeof(Arrays<int>::empty)==2*sizeof(int), "array type demand");
int main() { return Arrays<int>::data[2]+Arrays<int>::empty[0]-7; }
