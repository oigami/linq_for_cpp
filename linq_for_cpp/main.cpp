#include"linq_for_cpp.h"
#include<array>
#include<iostream>

int main() {
  using namespace std;
  int t[10];
  for (int i = 0; i < 10; i++) {
    t[i] = i;
  }
  using linq::holder::_1;
  auto foo = linq::From(t).Select([](int i) {return i * 2; });
  auto it = *foo.begin();
  ++it;
  std::istreambuf_iterator<char> a;
  const int size = sizeof(foo);
  for (auto i : foo) {
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
