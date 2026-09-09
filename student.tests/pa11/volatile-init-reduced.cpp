struct Device { volatile int status; };
int main() { Device array[1] = {{3}}; return array[0].status != 3; }
