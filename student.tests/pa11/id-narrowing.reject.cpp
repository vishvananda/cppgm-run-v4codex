// [dcl.init.list]/3,7: a nonconstant unsigned long to unsigned conversion narrows.
struct Id { unsigned value; };
Id recover(unsigned long storage) { return {storage}; }
