template<class...A> struct X{template<class...B>static bool test(){return (__is_same(A,B)&&...);}};
int main(){return X<int,long>::test<int>();}
