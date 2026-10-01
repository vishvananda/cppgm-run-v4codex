template<class T> struct box {};
namespace N {box(int)->box<int>;}
