template<class... T> auto left(T... x)->decltype((... + x)){return (...+x);}
template<class... T> auto right(T... x)->decltype((x + ...)){return (x+...);}
template<class... T> auto init_left(T... x)->decltype((1 + ... + x)){return (1+...+x);}
template<class... T> auto init_right(T... x)->decltype((x + ... + 1)){return (x+...+1);}
template int left<int,int>(int,int);
template int right<int,int>(int,int);
template int init_left<int,int>(int,int);
template int init_right<int,int>(int,int);
#ifndef ABI_PEER
int main(){return left(1,2)+right(1,2)+init_left(1,2)+init_right(1,2)-14;}
#endif
