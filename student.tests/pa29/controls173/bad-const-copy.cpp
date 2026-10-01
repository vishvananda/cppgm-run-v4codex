int main(){int x=2;auto f=[=]<class T>(T t){x+=t;return x;};return f(1);}
