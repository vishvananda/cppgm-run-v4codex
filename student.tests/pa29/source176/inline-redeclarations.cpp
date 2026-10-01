extern const int value;
inline const int value = 31;
extern const int value;
inline const int& reference = value;
inline int zero;
int main() { return reference + zero - 31; }
