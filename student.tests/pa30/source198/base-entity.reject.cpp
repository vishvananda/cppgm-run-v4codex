struct a { static int name; }; struct b { static int name; }; struct c : a,b { int get(){return name;} }; int main(){return c().get();}
