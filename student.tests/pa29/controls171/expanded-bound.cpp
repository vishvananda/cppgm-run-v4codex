template<unsigned... I> using selected=__type_pack_element<I...,int>;
selected<0> value=3;
int main(){return value-3;}
