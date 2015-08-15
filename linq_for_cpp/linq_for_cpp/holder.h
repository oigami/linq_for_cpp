#pragma once
#ifndef INCLUDE_LINQ_FOR_CPP_HOLDER_H
#define INCLUDE_LINQ_FOR_CPP_HOLDER_H
#include<type_traits>
namespace linq {

  namespace holder {
    struct BaseArgs {
      template<class T> using is_base = std::is_base_of<BaseArgs, T>;
    };

    template<class T> struct Data : BaseArgs {
      Data(const T &t) : data_(t) {}
      template<class Tmp> const T& operator()(const Tmp&) const { return data_; }
    private:
      Data operator=(const Data&) = delete;
      const T &data_;
    };

#define MakeMacro(MACRO_NAME, ...) MACRO_NAME(Comparator, == , __VA_ARGS__); \
                                   MACRO_NAME(Remainder,  %  , __VA_ARGS__); \
                                   MACRO_NAME(Adder,      +  , __VA_ARGS__); \
                                   MACRO_NAME(Subtractor, -  , __VA_ARGS__); \
                                   MACRO_NAME(Divider,    /  , __VA_ARGS__); \
                                   MACRO_NAME(Multiplier, *  , __VA_ARGS__);

#define MakeBeforeOperatorUnit(NAME, op, ...) template <class T, class T2> struct NAME; /* プロトタイプ宣言 */ 


    MakeMacro(MakeBeforeOperatorUnit);
#undef MakeBeforeOperatorUnit
#define MakeOperatorUnit(name, RETURN_NAME, OP) \
      template<class Value,class = typename std::enable_if<BaseArgs::is_base<Value>::value>::type> /* ValueがBaseArgsを継承してる場合のオペレータ */ \
        RETURN_NAME<name<T,T2>, Value> operator OP(const Value &t) const{\
          return RETURN_NAME<name<T,T2>, Value>(std::move(*this), std::move(t)); \
      } \
      template<class Value,class = typename std::enable_if<!BaseArgs::is_base<Value>::value>::type> /* ValueがBaseArgsを継承してない場合のオペレータ */ \
      RETURN_NAME<name<T,T2>, Data<Value>> operator OP(const Value &t) const{\
        return RETURN_NAME<name<T,T2>, Data<Value>>(std::move(*this), std::move(Data<Value>(t))); \
      }

#define MakeOperatorUnits(name) MakeOperatorUnit(name, Comparator, == );    \
                                MakeOperatorUnit(name, Remainder, %);       \
                                MakeOperatorUnit(name, Adder, +);           \
                                MakeOperatorUnit(name, Subtractor, -);      \
                                MakeOperatorUnit(name, Divider, / );        \
                                MakeOperatorUnit(name, Multiplier, *);


#define MakeComputingUnit(FUNC_NAME, op, ...)\
    template<class T, class T2> struct FUNC_NAME : BaseArgs {\
      private:\
      T param_; /* 第一引数 */ \
      T2 param2_; /* 第二引数 */\
      public:\
      FUNC_NAME(const T& param, const T2 &param2) : param_(param), param2_(param2) {}\
      FUNC_NAME(T&& param, T2 &&param2) : param_(std::move(param)), param2_(std::move(param2)) {}\
      template<class T3> /* 演算時のオペレータ */ \
      auto operator()(const T3& t)const\
       -> decltype(param_(t) op param2_(t)) { return (param_(t) op param2_(t)); }\
      MakeOperatorUnits(FUNC_NAME); \
      void operator=(const FUNC_NAME&) = delete; \
    };
    MakeMacro(MakeComputingUnit);

    struct Args : BaseArgs {
      template<class T> const T& operator()(const T &t) const { return t; }

#define MakeArgsOperator(name, op, ...) \
        /* Argsとの演算子の定義 TがArgsではない時 */ \
        template<class T> name<Args, Data<T>> operator op(const T &t) const{  \
          return name<Args, Data<T>>(std::move(*this), std::move(Data<T>(t)));\
        }\
        /* ArgsとArgsの演算子の定義  */ \
        name<Args, Args> operator op(const Args &t) const{ \
          return name<Args, Args>(*this, t); \
        }
      MakeMacro(MakeArgsOperator);
    }_1;

#undef MakeComputingUnit
#undef MakeOperatorUnits
#undef MakeOperatorUnit
#undef MakeMacro
  }
}

#endif // !INCLUDE_HOLDER_H