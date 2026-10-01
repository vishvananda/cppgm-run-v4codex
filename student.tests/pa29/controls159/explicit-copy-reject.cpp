struct Explicit { explicit operator int() const; };
static_assert(__reference_constructs_from_temporary(const int&,Explicit),"direct");
int main(){Explicit e; const int& r=e; return r;}
