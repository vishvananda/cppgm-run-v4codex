#include "lazy152.h"
Entry152* first152() { static Leaf152 leaf; return &leaf; }
Entry152* second152() { static Leaf152 leaves[2]; return &leaves[1]; }
