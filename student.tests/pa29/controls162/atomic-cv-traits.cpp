using A=_Atomic(int);static_assert(__is_same(__remove_const(const A),A),"");static_assert(__is_same(__remove_volatile(volatile A),A),"");int main(){}
