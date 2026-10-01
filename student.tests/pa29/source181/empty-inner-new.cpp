int made,dead;
struct E{E(){++made;}~E(){++dead;}};
int count(){return 2;}
int main(){auto p=new E[count()][0];delete[] p;return made||dead;}
