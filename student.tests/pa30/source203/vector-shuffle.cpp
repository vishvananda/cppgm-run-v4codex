typedef int V __attribute__((vector_size(16)));
int main(int argc,char**) {
 V a={10,20,30,40},b={50,60,70,80};
 V x=__builtin_shuffle(a,b,V{argc,-1,9,6});
 if(x[0]!=20||x[1]!=80||x[2]!=20||x[3]!=70)return 1;
 V y=__builtin_shuffle(a,V{5,6,7,8});
 return y[0]!=20||y[1]!=30||y[2]!=40||y[3]!=10;
}
