template<class... T> auto left(T... x)->decltype((... + x));
template<class... T> auto right(T... x)->decltype((x + ...));
template<class... T> auto init_left(T... x)->decltype((1 + ... + x));
template<class... T> auto init_right(T... x)->decltype((x + ... + 1));
int main(){return left(1,2)+right(1,2)+init_left(1,2)+init_right(1,2)-14;}
