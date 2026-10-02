struct Failure {};
int f() throw(Failure); int f() throw(Failure) {return 3;}
int g() throw(int,double); int g() throw(double,int,int) {return 4;}
int main(){return f()+g()-7;}
