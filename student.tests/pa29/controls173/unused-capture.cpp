int copies=0;
struct C { C(){} C(const C&){++copies;} int n; };
int main(){C c;auto f=[=]<class T>(T){(void)c;};return copies==1?0:1;}
