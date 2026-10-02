struct Count{operator int()const;operator unsigned long()const;};using P=decltype(new int[Count()]);
