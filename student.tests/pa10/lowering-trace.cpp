// Source-to-native trace for the independent architecture audit.
namespace N {
int step(int &value, int amount=2) { value+=amount; return value; }
}
int values[4]={1,2,3,4};
int *middle=&values[1];
extern "C" long power_difference(int *first,int *last) {return last-first;}
extern "C" long general_difference(char (*first)[3],char (*last)[3]) {return last-first;}
int main() {
    char rows[4][3];
    return N::step(*middle)==4 && power_difference(values+3,values+1)==-2 &&
        general_difference(rows+3,rows+1)==-2 ? 0:1;
}
