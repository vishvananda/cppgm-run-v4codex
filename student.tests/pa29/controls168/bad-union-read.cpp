union U{int a;long b;};constexpr U u={.b=7};static_assert(u.a==7,"inactive union member");
