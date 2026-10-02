struct Count{explicit operator int()const{return 3;}};void f(){new int[Count()];}
