using Z=int[0]; using U=int[];
static_assert(__is_same(__remove_extent(Z),int), "remove zero");
static_assert(__is_same(__remove_all_extents(Z),int), "remove all zero");
static_assert(__array_rank(Z)==1,"rank zero");
static_assert(!__is_array(Z) && !__is_bounded_array(Z) && !__is_unbounded_array(Z),"zero kind");
static_assert(__is_constructible(Z) && __is_destructible(Z),"lifetime");
int main(){return 0;}
