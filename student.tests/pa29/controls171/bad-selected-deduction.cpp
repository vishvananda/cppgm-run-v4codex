template<class T> int selected(__type_pack_element<0,T>){return 3;}
int main(){return selected(1)-3;}
