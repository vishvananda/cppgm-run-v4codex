// GNU frontend features used at ordinary hosted object boundaries.
#define CALL(fn, args...) fn(args)
#define STRING(args...) #args
#define SUM(head, args...) sum(head, ##args)
#define WRAP(args...) SUM(args)
int sum(int a, int b=0, int c=0) { return a+b+c; }
template<class T> using Selected = typename T::type;
template<class T> struct Derived : Selected<T> { int read() const { return this->value; } };
struct Base { int value = 17; };
struct Choice { using type = Base; };
static_assert(__is_same(decltype(__null),long), "GNU null has pointer-sized integral type");
static_assert(__null == 0, "GNU null is zero");
int* empty = __null;
enum __attribute__((__flag_enum__)) Flags { One __attribute__((unused))=1, Two=2 };
enum class __attribute__((unused)) Scoped : int { Value=3 };
static_assert(One+Two==static_cast<int>(Scoped::Value), "enum attributes preserve values");
int main(int argc, char** argv) {
    Derived<Choice> object;
    int (*compare)(const char*,const char*) = __builtin_strcmp;
    if (empty || object.read()!=17 || SUM(3)!=3 || WRAP(1,2,3)!=6) return 1;
    if (CALL(sum,4,5)!=9 || __builtin_strcmp(STRING(a, b),"a, b")) return 2;
    if (compare(argv[argc-1],argv[argc-1]) || __builtin_strncmp("abc","abd",2)) return 3;
    if (__builtin_strcmp("abc","abd")>=0 || __builtin_strncmp("abd","abc",3)<=0) return 4;
    return 0;
}
