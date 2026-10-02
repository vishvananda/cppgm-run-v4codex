struct Count{operator int()const=delete;};void f(){new int[Count()];}
