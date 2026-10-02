struct base {
    __float128 value;
    base(double x) : value(x) {}
};
struct number : base { number(double x) : base(x) {} };
double convert(double x) { number n(x); return double(n.value + 1); }
int main(int argc, char**) { return convert(argc) != argc + 1; }
