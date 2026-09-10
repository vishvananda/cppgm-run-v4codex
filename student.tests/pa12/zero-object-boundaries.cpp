int target=37;
struct Base { volatile long first; long second; };
union Choice { long first; unsigned char bytes[1024]; };
struct Reference { int& value=target; };
struct Owner : Base {
  Choice choice;
  Reference reference;
  Owner() : Base(),choice(),reference() {}
};
int main() {
  Owner owner;
  if(owner.first || owner.second || owner.choice.first) return 1;
  unsigned char const* p=reinterpret_cast<unsigned char const*>(&owner.choice);
  for(unsigned i=0; i<sizeof(Choice); ++i) if(p[i]) return 2;
  if(owner.reference.value!=37 || &owner.reference.value!=&target) return 3;
}
