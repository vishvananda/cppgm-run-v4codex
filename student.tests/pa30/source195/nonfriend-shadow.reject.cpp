struct base { typedef int type; };
struct owner { class base; base::type invalid; };
