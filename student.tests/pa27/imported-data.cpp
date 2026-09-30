struct Pair { long a; long b; };
extern Pair incoming[3];
extern double imported_fp;
extern int imported_count;
extern Pair* host_address();
int main()
{
    Pair* p = incoming + 2;
    p->a += 3;
    p->b = p->a + incoming[0].b;
    imported_fp += 2.5;
    ++imported_count;
    return p == host_address() && p->a == 20 && p->b == 24 &&
        imported_fp == 4.0 && imported_count == 8 ? 0 : 1;
}
