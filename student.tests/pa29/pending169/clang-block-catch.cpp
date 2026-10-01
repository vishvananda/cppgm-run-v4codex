// Host-only diagnostic: Clang 21.1.8 + libstdc++ on this host catches a block
// by value without loading the block reference from exception storage.
// Both the throw and catch are compiled by Clang; no student code is involved.
extern "C" {void* _NSConcreteStackBlock[32];}
int main(){int n=2;int (^b)(int)=^(int x){return n+x;};
 try{throw b;}catch(int (^c)(int)){return c(3)==5?0:1;}}
