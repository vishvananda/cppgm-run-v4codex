extern int calls;
extern int destroyed;
int next();
struct Object {
  int value;
  Object() : value(next()) {}
  ~Object() { ++destroyed; }
};
inline const char letters[4] = {'q','r','s','t'};
inline const int scalar = 11;
inline int dynamic = next();
inline Object object;
inline const Object& reference = Object();
inline thread_local int tls = next();
struct Members { inline static int value = next(); inline static const int constant = 19; };
template<class T> struct TemplateMembers { inline static int value = next(); };
template<class T> inline constexpr int variable = sizeof(T);
static inline int private_value = 17;
