struct alignas(64) Z{int a[0];};
struct S{char c;Z z;char d;};
int main(){Z z;S s;return (reinterpret_cast<unsigned long>(&z)%64)!=0||(reinterpret_cast<unsigned long>(&s.z)%64)!=0;}
