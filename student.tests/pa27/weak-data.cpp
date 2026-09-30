__attribute__((weak)) int replaceable = 1;
int* selected_address();
int main() { return replaceable == 13 && &replaceable == selected_address() ? 0 : 1; }
