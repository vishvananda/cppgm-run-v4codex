#include "support/extended_float.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
int main(){unsigned p;std::string s;while(std::cin>>p>>s){auto x=cppgm::parse_extended_float({s.data(),s.size()},p);unsigned long long a[2];std::memcpy(a,&x,16);if(p==11)std::cout<<std::hex<<cppgm::half_bits(x)<<'\n';else std::cout<<std::hex<<a[1]<<std::setfill('0')<<std::setw(16)<<a[0]<<'\n';}}
