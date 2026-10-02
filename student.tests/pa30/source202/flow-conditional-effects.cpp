int live,tests,arms;
struct Test{Test(){++live;} ~Test(){--live;} operator bool()const{++tests;return true;}};
int arm(){++arms;return live;}
int run(){if(Test() ? arm() : 0)return live==0?7:8;return 9;}
struct Choice{operator int()const{++arms;return 3;}};
int convert(int n){if(n ? Choice() : 0)return 4;return 5;}
int main(){return run()==7 && tests==1 && arms==1 && live==0 && convert(1)==4 && convert(0)==5 && arms==2?0:1;}
