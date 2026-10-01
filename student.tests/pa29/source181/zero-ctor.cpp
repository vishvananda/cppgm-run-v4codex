int calls;struct E{int a[0];E(){++calls;}~E(){++calls;}};int main(){{E a[3];}return calls!=6;}
