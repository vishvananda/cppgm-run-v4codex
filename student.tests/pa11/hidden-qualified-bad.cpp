namespace domain {
struct Value { friend int hidden(const Value&) { return 0; } };
}
int main() { domain::Value value; return domain::hidden(value); }
