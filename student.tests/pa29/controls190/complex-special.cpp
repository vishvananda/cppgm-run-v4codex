template<class T> constexpr T add(T a,T b) { return a+b; }
constexpr auto infinite=__builtin_complex(__builtin_inf(),1.0);
constexpr auto sum=add(infinite,infinite);
constexpr auto nan=__builtin_complex(__builtin_nan(""),2.0);
constexpr auto difference=nan-nan;
constexpr auto product=infinite*__builtin_complex(1.0,1.0);
constexpr auto quotient=infinite/__builtin_complex(1.0,1.0);
constexpr auto wide=__builtin_complex(__builtin_infl(),1.0L)+__builtin_complex(1.0L,2.0L);
constexpr auto narrow=__builtin_complex(__builtin_inff(),1.0f)+__builtin_complex(1.0f,2.0f);
static_assert(__builtin_isinf(__real__ sum)&&__imag__ sum==2,"constant infinity");
static_assert(__builtin_isnan(__real__ difference)&&__imag__ difference==0,"constant NaN");
static_assert(__builtin_isinf(__real__ product)&&__builtin_isinf(__imag__ product),"multiply recovery");
static_assert(__builtin_isinf(__real__ quotient)&&__builtin_isinf(__imag__ quotient),"divide recovery");
static_assert(__builtin_isinf(__real__ narrow)&&__imag__ narrow==3,"float");
static_assert(__builtin_isinf(__real__ wide)&&__imag__ wide==3,"long double");
int main(int argc,char**) {
  auto a=__builtin_complex(__builtin_inf(),double(argc));
  auto b=add(a,a);
  return __builtin_isinf(__real__ b)&&__imag__ b==__imag__ sum ? 0:1;
}
