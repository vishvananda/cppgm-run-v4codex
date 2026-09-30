extern int source_value(int);
__attribute__((noinline)) inline int checked_value(int n)
{
    try { if (n) throw source_value(n); return 0; }
    catch (int value) { return value + 3; }
}
