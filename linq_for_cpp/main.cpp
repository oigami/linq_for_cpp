#include"linq_for_cpp.h"
#include<array>
#include<iostream>
#include<memory>
using namespace linq::lambda;
struct Hoge {
  template<class Type, class T>
  linq::lambda::Operator<TypeVal<Type, T>, linq::lambda::Data<int>, linq::lambda::Multiplier> operator->*(T Type::*a) {
    TypeVal<Type, T> type(a);
    using namespace linq::lambda;
    return Operator<TypeVal<Type, T>, Data<int>, Multiplier>(type, 10, Multiplier());
  }
  template<class Type, class T>
  linq::lambda::Operator<TypeVal<Type, T>, linq::lambda::Data<int>, linq::lambda::Multiplier> val() {
    TypeVal<Type, T> type(a);
    using namespace linq::lambda;
    return Operator<TypeVal<Type, T>, Data<int>, Multiplier>(type, 10, Multiplier());
  }
  template<class Class, class T, T(Class::*M)()>
  T test() {

  }
};
template<class Class, class T>
T Val(T Class::*val) {

}
template<class Class, class T, T(Class::*M)()>
T Val2() {

}
struct X {
  X(int i) :x(i) {}
  int x;
};
int main() {
  using namespace std;
  using linq::lambda::_1;
  int t[10];
  for (int i = 0; i < 10; i++) {
    t[i] = i;
  }
  X* x;
  auto xx = &X::x;
  x->*xx;
  std::shared_ptr<X> s;
  s.get()->*xx;
  std::vector<X*>vec{};
  vec.emplace_back();
  for (const auto& i : linq::From(vec).Where(_1[&X::x] % 2)) {
    cout << i->x << endl;
  }
  //constexpr auto f2 = _1 % 2;
  //sizeof(f2);
  const int aa = 1 % 2;
  auto f = [&](int i) { return i * 2; };
  auto foo = linq::From(t).Where(2 * _1 % 2 * 0).Where(_1 % 3).Select([](int i) {return i * 2; });
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
