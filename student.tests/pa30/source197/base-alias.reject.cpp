struct left { typedef int name; };
struct right { typedef int name; };
struct both : left,right { name value; };
