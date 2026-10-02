struct Bad { Bad() = delete; ~Bad() = delete; };
union Choice { int value; Bad unused; ~Choice() {} };
int main() { Choice c={7}; return c.value-7; }
