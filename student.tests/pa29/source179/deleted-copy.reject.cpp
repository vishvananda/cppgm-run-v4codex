struct Pair{int a,b;Pair(){}Pair(const Pair&)=delete;};int main(){Pair p;auto [a,b]=p;}
