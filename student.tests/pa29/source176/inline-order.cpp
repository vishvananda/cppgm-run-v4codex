int count;
int next() { return ++count; }
template<class T> struct Nodes {
 inline static int first = next();
 inline static int second = first + next();
 inline static int unused = next();
};
int main() { return Nodes<int>::second == 3 && count == 2 ? 0 : 1; }
