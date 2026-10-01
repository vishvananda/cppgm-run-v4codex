constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int parameter(int p,int q){const int& r=active()?p:q;return r;}
constexpr int parameter_write(int p,int q){int& r=active()?p:q;r=11;return p;}
int main(){int a[]={parameter(3,7),parameter_write(3,7)};
return parameter(3,7)==3&&parameter_write(3,7)==11&&a[0]==3&&a[1]==11?0:1;}
