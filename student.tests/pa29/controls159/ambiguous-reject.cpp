struct Ambiguous { operator unsigned int() const; operator long() const; };
static_assert(!__reference_constructs_from_temporary(const int&,Ambiguous),"ambiguous");
int main(){Ambiguous a; const int& r(a); return r;}
