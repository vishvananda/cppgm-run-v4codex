int range_live, iterator_live, destroyed;
struct Iterator {
    int* pointer;
    Iterator(int* p) : pointer(p) { ++iterator_live; }
    Iterator(const Iterator& other) : pointer(other.pointer) { ++iterator_live; }
    ~Iterator() { --iterator_live; }
    int& operator*() { return *pointer; }
    Iterator& operator++() { ++pointer; return *this; }
    bool operator!=(const Iterator& other) { return pointer != other.pointer; }
};
struct Range {
    int values[2];
    Range() { ++range_live; values[0]=3; values[1]=4; }
    ~Range() { --range_live; ++destroyed; }
    Iterator begin() { return Iterator(values); }
    Iterator end() { return Iterator(values+2); }
};
template<int Bias> int fixed(Range& range) {
    int total=Bias;
    for (const auto& value : range) total+=value;
    return total;
}
template<class T> int temporary() {
    int total=0;
    for (const auto& value : T()) {
        if (range_live!=1 || iterator_live!=2) return -1;
        if (value==3) continue;
        total+=value;
    }
    return total;
}
int main() {
    if (temporary<Range>()!=4 || temporary<Range>()!=4) return 1;
    if (range_live || iterator_live || destroyed!=2) return 2;
    {
        Range range;
        if (fixed<3>(range)!=10 || fixed<5>(range)!=12 || fixed<3>(range)!=10) return 3;
        if (iterator_live) return 4;
    }
    return range_live || iterator_live || destroyed!=3;
}
