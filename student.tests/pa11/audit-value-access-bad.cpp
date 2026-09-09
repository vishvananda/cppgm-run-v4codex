class Hidden { Hidden() {} friend int allowed(); };
struct Box { Hidden value; };
int allowed() { Box a[2]={}; return 0; }
int main() { Box b[2]={}; }
