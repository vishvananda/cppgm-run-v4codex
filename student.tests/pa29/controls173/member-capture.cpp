struct C { int n;int method(){return n;}
 int run(){auto f=[this]<class T>(T x){return method()+x;};return f(4);}
};int main(){C c={3};return c.run()==7?0:1;}
