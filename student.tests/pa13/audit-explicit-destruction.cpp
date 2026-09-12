void operator delete(void*) noexcept;
// N3485 [class.dtor]/13: an explicit unqualified destructor call can be virtual.
int trace;
struct B { virtual ~B() { trace = trace * 10 + 1; } };
struct D : B {
    ~D() override { trace = trace * 10 + 2; }
    virtual void destroy() { this->~D(); }
};
struct E : D { ~E() override { trace = trace * 10 + 3; } };
int main() {
    D* complete = new D;
    B* base = complete;
    base->~B();
    ::operator delete(complete);
    if (trace != 21) return 1;
    trace = 0;
    complete = new D;
    base = complete;
    base->B::~B();
    ::operator delete(complete);
    if (trace != 1) return 2;
    trace = 0;
    E* further = new E;
    further->destroy();
    ::operator delete(further);
    return trace != 321;
}
