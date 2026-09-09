class Owner {
    static int select(int);
public:
    static int select(long);
};
// Access is checked on the best candidate; it does not filter the set.
int main() { return Owner::select(0); }
