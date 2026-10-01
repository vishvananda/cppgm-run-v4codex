int calls;
struct E { int a[0]; E(){++calls;} ~E(){++calls;} };
int main(){E* p=new E[3];int constructed=calls;delete[] p;return constructed!=3||calls!=6;}
