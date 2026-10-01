int made,dead;
struct E{int a[0];E(){if(++made==3)throw 7;}~E(){++dead;}};
int main(){try{E* p=new E[4];delete[] p;}catch(int n){return n!=7||made!=3||dead!=2;}return 2;}
