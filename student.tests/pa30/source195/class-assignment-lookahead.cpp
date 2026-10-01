struct tag {};
template<class... T> struct box;
template<class T> struct pair {
    template<class... U, class... V> pair(tag, box<U...>, box<V...>);
};
template<class... T> struct box {
    box() {}
    box(const box&) = default;
    template<class U> box& operator=(const U&) { return *this; }
};
template<class T> template<class... U, class... V>
pair<T>::pair(tag, box<U...>, box<V...>) {}
int main() { box<int> a; a = 7; box<int> b(a); return sizeof(b) != 1; }
