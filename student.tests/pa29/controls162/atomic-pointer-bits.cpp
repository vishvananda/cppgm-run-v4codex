int main(){int values[8];int* p=values;return __atomic_or_fetch(&p,0L,5)==values?0:1;}
