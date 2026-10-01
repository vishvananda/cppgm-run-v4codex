struct Pair{int a,b;};int main(){Pair p{1,2};const auto& [a,b]=p;a=4;}
