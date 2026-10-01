int alive=0;
struct Life { Life(){++alive;} Life(const Life&){++alive;} ~Life(){--alive;} };
int main(){
 { Life x; auto f=[=]<class T>(T n){(void)x;Life y;if(n)throw 7;return alive;};
   if(alive!=2 || f(0)!=3 || alive!=2)return 1;
   try{f(1L);return 2;}catch(int v){if(v!=7 || alive!=2)return 3;}
 }
 return alive==0?0:4;
}
