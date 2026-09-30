#include <string>
std::string choose(bool first,bool second) {
    const std::string left=first ? "left" : std::string("right")+std::string("tail");
    const std::string right=second ? "up" : std::string("down")+std::string("tail");
    return left+right;
}
int main() {
    if (choose(false,false)!="righttaildowntail") return 1;
    if (choose(false,true)!="righttailup") return 2;
    if (choose(true,false)!="leftdowntail") return 3;
    if (choose(true,true)!="leftup") return 4;
    std::string long_value("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    std::string copy(long_value);
    long_value+=copy;
    return long_value.size()==124 && long_value.substr(62)==copy ? 0:5;
}
