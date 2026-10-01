#include <thread>
#include <vector>
extern "C" {
void ordinary_increment(int*);int increment(int*);void bits(unsigned*,unsigned);int acquire(const int*);void release(int*,int);
struct Pair { unsigned long low,high; };Pair get_pair(const Pair*);void set_pair(Pair*,Pair);
}
int main(){
 int ordinary=0;int count=0;unsigned mask=0;std::vector<std::thread> threads;
 for(unsigned j=0;j<4;++j)threads.emplace_back([&,j]{for(int i=0;i<40000;++i){ordinary_increment(&ordinary);increment(&count);bits(&mask,1u<<j);}});
 for(auto& t:threads)t.join();if(count!=160000 || ordinary!=160000 || mask!=15)return 1;
 int ready=0,payload=0;std::thread sender([&]{payload=123;release(&ready,1);});
 while(!acquire(&ready)){} if(payload!=123)return 2;sender.join();
 alignas(16) Pair pair={3,~3ul};int finished=0;
 std::thread writer([&]{for(unsigned long i=0;i<40000;++i)set_pair(&pair,{i,~i});release(&finished,1);});
 bool good=true;while(!acquire(&finished)){auto v=get_pair(&pair);good&=v.high==~v.low;}writer.join();
 return good?0:3;
}
