int copies=0;
struct C {C(){} C(const C&){++copies;}};
int main(){C c;auto f=[=]<class T>(T){return sizeof(c)+sizeof(T);};if(copies)return 1;return f(1)==sizeof(C)+sizeof(int)?0:2;}
