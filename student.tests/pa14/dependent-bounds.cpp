// N3485 [dcl.array], [temp.dep.constexpr], [temp.deduct.type]: a bound can
// depend on a type parameter; it remains part of a reference-to-array type.
template<class T> int sum(T (&array)[sizeof(T)]) {
    T result=0;
    for (unsigned long i=0;i<sizeof(T);++i) result+=array[i];
    return result;
}
template<class T> int renamed(int (&array)[sizeof(T)+1]);
template<class U> int renamed(int (&array)[sizeof(U)+1]) { return array[sizeof(U)]; }
template<class T> struct Bounds {
    using Array=T[sizeof(T)+1];
    T data[sizeof(T)+1];
    unsigned long run() {
        Array local={};
        return sizeof(local)+sizeof(data);
    }
};
template<class T> unsigned long local() {
    using Array=int[(sizeof(T)+1)*2];
    Array array={};
    return sizeof(array);
}
struct Three { static const int count=3; };
struct Five { enum { count=5 }; };
template<class T> int counted(int (&array)[T::count]) { return array[T::count-1]; }
template<class T> unsigned long row(T (&array)[2][sizeof(T)]) { return sizeof(array); }
template<class T> unsigned long aligned(int (&array)[__alignof(T)]) { return sizeof(array); }
template<class T> unsigned long parameter(T value,int (&array)[sizeof(value)]) { return sizeof(array); }
template<class T> int choose(int (&array)[sizeof(T)==4 ? 3 : 5]);
template<class U> int choose(int (&array)[sizeof(U)==4 ? 3 : 5]) { return sizeof(array); }
template<class T> int casted(int (&array)[static_cast<int>(sizeof(T))+1]) { return sizeof(array); }
template<class T> int ccast(int (&array)[(int)sizeof(T)+1]) { return sizeof(array); }
template<class T> int functional(int (&array)[int(sizeof(T))+1]) { return sizeof(array); }
template<class T> int shorted(int (&array)[sizeof(T) ? 3 : 1/0]) { return sizeof(array); }
template<class T> int logical(int (&array)[(sizeof(T) || 1/0) + !(sizeof(T) && 0)]) { return sizeof(array); }
template<class T> int adjusted(int array[sizeof(T)],int (&other)[sizeof(array)]) { return array[0]+sizeof(other); }
int main() {
    int four[4]={1,2,3,4}; long eight[8]={1,2,3,4,5,6,7,8};
    if (sum(four)!=10 || sum(eight)!=36) return 1;
    int five[5]={1,2,3,4,5}; int nine[9]={1,2,3,4,5,6,7,8,9};
    if (renamed<int>(five)!=5 || renamed<long>(nine)!=9) return 2;
    Bounds<int> a; Bounds<long> b;
    if (a.run()!=40 || b.run()!=144 || local<int>()!=40 || local<long>()!=72) return 3;
    int three[3]={1,2,3};
    if (counted<Three>(three)!=3 || counted<Five>(five)!=5) return 4;
    int matrix[2][4]={};
    int two[2]={}; int pointers[8]={};
    if (choose<int>(three)!=12 || choose<long>(five)!=20 || casted<int>(five)!=20 ||
        ccast<int>(five)!=20 || functional<int>(five)!=20 || shorted<int>(three)!=12 ||
        logical<int>(two)!=8 || adjusted<int>(four,pointers)!=33) return 5;
    return row(matrix)!=32 || aligned<int>(four)!=16 || parameter(2,four)!=16;
}
