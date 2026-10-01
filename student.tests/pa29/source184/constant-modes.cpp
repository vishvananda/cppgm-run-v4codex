constexpr int data=23;
constexpr int read(bool active, const int *p) {
 return active ? *(int*)p : *reinterpret_cast<const int*>(p);
}
static_assert(read(true,&data)==23,"only evaluated branch matters");
constexpr int *pointer=(int*)&data;
static_assert(pointer==&data,"stored address");
constexpr int through(const int *p) {return *(int*)p;}
static_assert(through(pointer)==23,"stored address argument");
int main(int argc,char**) {int value=argc;return read(false,&value)==argc?0:1;}
