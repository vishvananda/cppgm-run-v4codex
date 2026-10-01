constexpr bool active(){return __builtin_is_constant_evaluated();}
struct Box {int n; Box():n(active()?1:2){} };
struct Forward {int n; Forward(int k):n(k){} };
struct Ctor {int n; constexpr Ctor(int k):n(k){} };
Box global; Box braced{};
Forward forward(active()?3:4); Forward listed{active()?3:4};
int dynamic_value(){return active()?5:6;}
Ctor failed_trial(active()?dynamic_value():7);
int main(){static Box local; Box automatic;
return global.n==2&&local.n==2&&automatic.n==2&&forward.n==4&&failed_trial.n==7&&braced.n==2&&listed.n==4?0:1;}
