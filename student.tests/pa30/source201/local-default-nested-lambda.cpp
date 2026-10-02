int f(int x=[]()->int{int y=3;return [&]{return y;}();}()){return x;} int main(){return f()-3;}
