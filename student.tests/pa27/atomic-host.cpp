#include <thread>
extern "C" long increment(volatile long*);
extern "C" int controls();
int main() {
    if (controls()) return 1;
    volatile long value=0;
    long sums[4]={};
    std::thread workers[4];
    for (int i=0;i<4;++i) workers[i]=std::thread([&,i]{
        for (int n=0;n<50000;++n) sums[i]+=increment(&value);
    });
    for (auto& worker:workers) worker.join();
    return value!=200000 || sums[0]+sums[1]+sums[2]+sums[3]!=19999900000L;
}
