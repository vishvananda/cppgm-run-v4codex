typedef decltype(sizeof(0)) size_t;
void* operator new(size_t,void* p) noexcept {return p;}
struct Late { long value=66; Late(); };
Late::Late()=default;
struct Early { long value; Early()=default; };
Late* make_late(void* storage) {return new(storage) Late();}
Early* make_early(void* storage) {return new(storage) Early();}
int main() {
  alignas(8) unsigned char storage[8];
  Late* late=make_late(storage);
  if(late->value!=66)return 1;
  Early* early=make_early(storage);
  return early->value!=0;
}
