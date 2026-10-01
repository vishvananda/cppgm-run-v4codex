template<class T> struct Pair { T a; T b; };
template<class T,int N> int sum(Pair<T> (&arr)[N]) {
 int total=0; for(auto [x,y]:arr) total+=x+y; return total;
}
int main(){ Pair<int> arr[3]={{1,2},{3,4},{5,6}}; if(sum(arr)!=21)return 1;
 for(auto& [x,y]:arr){x+=y;y=0;} return arr[0].a!=3 || arr[2].a!=11 || sum(arr)!=21;
}
