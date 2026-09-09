struct Inner { int x; int y; };
struct Outer { Inner pairs[2]; char text[4]; int tail; };
Outer global = {1,2,3,4,"ok",9};
char text[] = "hello";
unsigned char bytes[] = "yes";
struct Big { int values[10000]; };
int main() {
    Outer local = {5,6,{7},"hi",11};
    if (local.pairs[0].x != 5 || local.pairs[0].y != 6 || local.pairs[1].x != 7 || local.pairs[1].y != 0) return 1;
    if (local.text[0] != 'h' || local.text[2] != 0 || local.tail != 11) return 2;
    if (global.pairs[1].y != 4 || global.text[1] != 'k' || global.tail != 9) return 3;
    Big big = {};
    if (big.values[0] != 0 || big.values[9999] != 0) return 4;
    volatile int volatile_values[100] = {7};
    if (volatile_values[0] != 7 || volatile_values[99] != 0) return 5;
    return text[4] != 'o' || bytes[1] != 'e';
}
