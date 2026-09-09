struct Bits { unsigned value:3; };
int main() {
  Bits bits={0};
  return (bits.value-1)/2 != 0;
}
