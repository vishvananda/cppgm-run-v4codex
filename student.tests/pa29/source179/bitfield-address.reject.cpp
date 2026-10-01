struct Bits{unsigned a:3;unsigned b:4;};int main(){Bits p{1,2};auto& [a,b]=p;auto q=&a;}
