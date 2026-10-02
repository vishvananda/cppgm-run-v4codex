class Item { ~Item() {} };
union Choice { Item item; int unused; ~Choice() {} };
void f() { Choice c = {{}}; }
