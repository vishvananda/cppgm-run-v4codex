#include <thread>
extern "C" void update(int*);
extern "C" int concurrent() {
 int n=0;auto f=[&]{for(int i=0;i<50000;++i)update(&n);};
 std::thread a(f),b(f),c(f);a.join();b.join();c.join();return n!=150000;
}
