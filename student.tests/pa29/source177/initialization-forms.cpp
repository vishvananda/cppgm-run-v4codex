int main() {
  int sum=0;
  if (typedef int T; T x=2) sum+=x;
  if (int a[2]={1,2}; a[0]) sum+=a[1];
  if (static int n=3; n) sum+=n;
  if (int (*p)[2]=0; !p) ++sum;
  return sum==8 ? 0 : 1;
}
