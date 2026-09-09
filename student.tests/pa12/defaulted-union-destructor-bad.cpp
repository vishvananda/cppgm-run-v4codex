struct Field { ~Field() {} };
union U { Field field; ~U(); };
U::~U() = default;
