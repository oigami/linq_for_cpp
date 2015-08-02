#include"linq_for_cpp.h"

int main() {
  using namespace std;
  int t[10];
  for (int i = 0; i < 10; i++) {
    t[i] = i;
  }
  using linq::holder::_1;
  auto foo = linq::From(t).Where(_1 % 2 == 0).Select(_1 * 2).ToVector();
  for (auto& i : foo) {
    cout << i << endl;
  }
  /*
  output:
  0
  4
  8
  12
  16
  */
  return 0;
}
