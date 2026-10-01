extern "C" int puts(const char*);
struct C { C(){puts(__PRETTY_FUNCTION__);} ~C(){puts(__PRETTY_FUNCTION__);} operator int() const {puts(__PRETTY_FUNCTION__);return 1;} int operator()() const {puts(__PRETTY_FUNCTION__);return 0;}};
auto inferred(){return __PRETTY_FUNCTION__;}
int main(){C c;puts(inferred());return c()+int(c)-1;}
