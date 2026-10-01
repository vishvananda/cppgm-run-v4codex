int main(){int a[2]={3,4};auto& [x,y]=a;x=8;const auto& [c,d]=a;static_assert(__is_same(decltype(c),const int),"element cv");return a[0]!=8||d!=4;}
