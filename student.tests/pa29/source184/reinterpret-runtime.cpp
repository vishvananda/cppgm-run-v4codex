int main(){
 const unsigned int n=7;
 unsigned char& byte=(unsigned char&)n;
 const unsigned int *p=&n;
 const unsigned char* bytes=reinterpret_cast<const unsigned char*>(p);
 int value=11; int *q=&value; int **pp=&q;
 const int* const *qualified=reinterpret_cast<const int*const*>(pp);
 return byte==7 && *bytes==7 && **qualified==11?0:1;
}
