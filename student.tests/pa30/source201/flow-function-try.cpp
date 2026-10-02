int f(int x) try {if(x)throw 1;return 2;}catch(...){return 3;} int main(){return f(0)+f(1)-5;}
