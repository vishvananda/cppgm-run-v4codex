struct a { typedef int name; }; struct b { typedef long name; }; struct c : a,b { name value; };
