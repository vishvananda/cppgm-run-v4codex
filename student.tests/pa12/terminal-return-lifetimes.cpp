int live, shells, checked;
struct Item {
  int n;
  Item(int value):n(value){++live;}
  Item(Item const& other):n(other.n){++live;}
  Item(Item&& other):n(other.n){++live;other.n=-1;}
  ~Item(){--live;}
};
struct Shell {
  Item value;
  Shell(int n):value(n){++shells;}
  ~Shell(){--shells;}
};
Item choose(bool a,bool b) {
  Item local(7);
  return a ? (b ? local : Shell(9).value) : Shell(11).value;
}
void observe(){++checked;}
struct Checked {
  int n;
  Checked(int value):n(value){++live;}
  Checked(Checked const& other):n(other.n){observe();++live;}
  ~Checked(){--live;}
};
struct CheckedShell {Checked value; CheckedShell(int n):value(n){++shells;}~CheckedShell(){--shells;}};
Checked choose_checked(bool a) {Checked local(13); return a ? local : CheckedShell(17).value;}
int main() {
  {Item x=choose(true,true);if(x.n!=7||live!=1||shells)return 1;}
  {Item x=choose(true,false);if(x.n!=9||live!=1||shells)return 2;}
  {Item x=choose(false,false);if(x.n!=11||live!=1||shells)return 3;}
  {Checked x=choose_checked(true);if(x.n!=13||live!=1||shells||checked!=1)return 4;}
  {Checked x=choose_checked(false);if(x.n!=17||live!=1||shells)return 5;}
  return live||shells;
}
