constexpr int n=1;
static_assert((const unsigned&)n==1,"static conversion creates a temporary");
static_assert((const double&)n==1.,"scalar temporary conversion");
int main(){int value=7; const unsigned& u=(const unsigned&)value;
 value=9; return u==7 && (const double&)value==9.?0:1;}
