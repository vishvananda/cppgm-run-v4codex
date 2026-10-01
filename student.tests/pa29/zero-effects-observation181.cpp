extern "C" int printf(const char*,...);
int made,dead;
struct E{int a[0];E(){++made;}~E(){++dead;}};
int main(){
  {E local[3];}
  printf("%d %d\n",made,dead);
  made=dead=0;E* p=new E[3];delete[] p;
  printf("%d %d\n",made,dead);
}
