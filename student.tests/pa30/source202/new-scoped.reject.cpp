enum class E{a=3};struct Count{operator E()const;};using P=decltype(new int[Count()]);
