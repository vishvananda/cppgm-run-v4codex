// Fixed call facts share source inputs; actual local objects and temporaries
// keep distinct identities and lifetime actions in each specialization.
struct Item {
    static int alive, total;
    int value;
    Item(int n):value(n) { ++alive; }
    Item(const Item&)=delete;
    Item(Item&& other):value(other.value) { other.value=0; ++alive; }
    ~Item() { total+=value; --alive; }
};
int Item::alive=0; int Item::total=0;
int touch(int& value,int delta=2) { value+=delta; return value; }
int observe(const Item& value) { return value.value; }
template<class T> int flow(int n) {
    int local=n;
    int a=touch(local);
    const Item& value=n ? Item(local) : Item(7);
    int b=observe(value);
    int c=observe(Item(local+1));
    if (Item::alive!=1) return -1;
    return a+b+c+sizeof(T);
}
int increment(int& n) { return ++n; }
template<class T> int indirect(int(*function)(int&),int n) {
    int local=n;
    int value=function(local);
    return value+local+sizeof(T);
}
int main() {
    if (flow<int>(3)!=20 || Item::alive || Item::total!=11) return 1;
    if (flow<long>(0)!=20 || Item::alive || Item::total!=21) return 2;
    return indirect<int>(increment,4)!=14 || indirect<long>(increment,8)!=26;
}
