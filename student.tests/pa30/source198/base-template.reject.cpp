struct a { template<class T> using name=T; }; struct b { template<class T> using name=T; }; struct c:a,b { name<int> value; };
