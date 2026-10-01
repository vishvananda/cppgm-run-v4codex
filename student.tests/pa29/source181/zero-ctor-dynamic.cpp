int made,dead,bounds;
struct E{int a[0];E(){++made;}~E(){++dead;}};
int n(){++bounds;return 3;}
int main(){E* p=new E[n()];delete[] p;return made!=3||dead!=3||bounds!=1;}
