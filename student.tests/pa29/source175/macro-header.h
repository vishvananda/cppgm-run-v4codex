#define INNER_LINE __builtin_LINE()
#define OUTER_LINE INNER_LINE
#define CALL_SITE loc()
#define FILE_SITE __builtin_FILE()
constexpr int loc(int line=__builtin_LINE()){return line;}
