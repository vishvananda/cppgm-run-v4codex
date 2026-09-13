// N3485 [dcl.enum], [temp.dep.type], [temp.mem], [temp.inst].
// Member enum types retain current-instantiation identity through signatures,
// renamed definition heads, nested qualifiers and anonymous-enum aliases.
template<class T> struct Enums {
    enum E : unsigned char { first=sizeof(T), second=first+1 };
    enum class Scoped : unsigned short { first=3, second=4 };
    enum { count=3, more=count+1 };
    typedef decltype(count) Anonymous;
    E echo(E value) { return value; }
    Scoped echo(Scoped value) { return value; }
    Anonymous echo(Anonymous value) { return value; }
    int score(E);
    struct Nested {
        enum Inner { first=4, second=5 };
        Inner echo(Inner);
    };
};
template<class U> int Enums<U>::score(E value) { return int(value)+sizeof(U); }
template<class U> typename Enums<U>::Nested::Inner Enums<U>::Nested::echo(Inner value) { return value; }
int select(Enums<int>::E) { return 4; }
int select(Enums<long>::E) { return 8; }
int main() {
    Enums<int> a; Enums<long> b;
    if (a.echo(Enums<int>::first)!=sizeof(int) || b.echo(Enums<long>::first)!=sizeof(long)) return 1;
    if (a.echo(Enums<int>::Scoped::second)!=Enums<int>::Scoped::second) return 2;
    if (b.echo(Enums<long>::Scoped::first)!=Enums<long>::Scoped::first) return 3;
    if (a.echo(Enums<int>::count)!=3 || b.echo(Enums<long>::more)!=4) return 4;
    if (a.score(Enums<int>::second)!=2*sizeof(int)+1 || b.score(Enums<long>::second)!=2*sizeof(long)+1) return 5;
    Enums<int>::Nested nested; Enums<long>::Nested other;
    if (nested.echo(Enums<int>::Nested::first)!=4 || other.echo(Enums<long>::Nested::second)!=5) return 6;
    if (select(a.echo(Enums<int>::first))!=4 || select(b.echo(Enums<long>::first))!=8) return 7;
    return 0;
}
