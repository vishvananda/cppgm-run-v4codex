struct Box { int &value; };
int main() { int x=1; int y=2; Box a={x}; Box b={y}; a=b; }
