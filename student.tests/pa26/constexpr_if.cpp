int dead;
struct Guard { ~Guard(){++dead;} };
template<bool B,class T> int choose(T x) {
    Guard g;
    if constexpr(B) { return x+1; }
    else { typename T::not_a_type bad; return bad; }
}
template<bool B,class T> int other(T x) {
    if constexpr(B) return x.not_a_member();
    else { Guard g;return x+2; }
}
int main(){return choose<true>(3)==4 && other<false>(4)==6 && dead==2 ? 0:1;}
