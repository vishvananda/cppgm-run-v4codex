extern "C" int quad_truth(__float128);
extern "C" int half_truth(_Float16);
int main() {
  if(quad_truth(0.0Q)||quad_truth(-0.0Q)||half_truth(0.0F16)||half_truth(-0.0F16))return 1;
  if(!quad_truth(0x1p-16494Q)||!half_truth(0x1p-24F16))return 2;
  if(!quad_truth((__float128)__builtin_nan(""))||!half_truth((_Float16)__builtin_nan("")))return 3;
  return 0;
}
