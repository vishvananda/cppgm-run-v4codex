// N3485 3.6.2 [basic.start.init]/2 requires selected's constant initialization
// before observed's dynamic initializer calls read(). A zero reference fails.
int read();
int observed = read();
constexpr int value = 11;
struct Source {
  template<class T> constexpr operator const T&() const { return value; }
};
static constexpr auto& selected = true ? Source() : value;
int read() { return selected; }
int main() { return observed != 11; }
