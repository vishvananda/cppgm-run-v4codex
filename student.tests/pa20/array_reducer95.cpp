// The PA16 automatic-array rule also applies when PA20 uses the objects.
// Two equal initial images must still initialize distinct mutable arrays.
int main() {
    int first[3] = {1, 2};
    int second[3] = {1, 2};
    for (int& value : first) ++value;
    return first == second || first[0] != 2 || first[1] != 3 || first[2] != 1 ||
           second[0] != 1 || second[1] != 2 || second[2] != 0;
}
