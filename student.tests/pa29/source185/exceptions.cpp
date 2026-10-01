using S = _BitInt(9);
using U = unsigned _BitInt(93);
int destroyed;
struct Guard { ~Guard() { ++destroyed; } };
void raise(int x) { Guard g; if(x) throw S(-17); throw U(1)<<83; }
int main() {
 try { raise(1); } catch(S x) { if(x!=-17 || destroyed!=1) return 1; }
 try { raise(0); } catch(S) { return 2; } catch(U x) { if(x!=(U(1)<<83) || destroyed!=2) return 3; }
 return 0;
}
