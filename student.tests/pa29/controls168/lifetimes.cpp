int alive;
struct Element {int value;Element(int x):value(x){++alive;} ~Element(){--alive;}};
struct S { int tag; Element e; };
int read(const S& s){return alive==1?s.e.value:-1;}
int main(){
 int v=read((S){.e=7});if(v!=7 || alive!=0)return 1;
 {const S& s=(S){.e=9};if(alive!=1 || s.tag!=0 || s.e.value!=9)return 2;}
 return alive!=0;
}
