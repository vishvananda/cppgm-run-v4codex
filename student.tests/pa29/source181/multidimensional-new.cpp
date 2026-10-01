int made,dead;
struct E{int a[0];E(){++made;}~E(){++dead;}};
int count(){return 2;}
int main(){auto p=new E[count()][3];delete[] p;return made!=6||dead!=6;}
