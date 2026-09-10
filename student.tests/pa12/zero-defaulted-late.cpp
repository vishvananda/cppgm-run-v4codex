typedef decltype(sizeof(0)) size_t;
void* operator new(size_t,void* p) noexcept {return p;}
struct Late { long value; Late(); };
Late::Late()=default;
struct Early { long value; Early()=default; };
int main() {
  alignas(8) unsigned char storage[8];
  for(int j=0;j<8;++j)storage[j]=66;
  Late* late=new(storage) Late();
  unsigned char const* bytes=reinterpret_cast<unsigned char const*>(late);
  for(int j=0;j<8;++j)if(bytes[j]!=66)return 1;
  Early* early=new(storage) Early();
  return early->value!=0;
}
