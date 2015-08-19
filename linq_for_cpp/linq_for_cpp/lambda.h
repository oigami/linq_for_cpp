#pragma once
#ifndef INCLUDE_LINQ_FOR_CPP_HOLDER_H
#define INCLUDE_LINQ_FOR_CPP_HOLDER_H
#if _MSC_VER >=1900 
#define LINQ_HOLDER_USE_CONSTEXPR
#endif
#ifdef LINQ_HOLDER_USE_CONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR 
#endif

#include<type_traits>
#include<memory>
//#pragma warning(disable: 4579) 
namespace linq {
  namespace lambda {
    struct BaseArg {};

    template<class T1, class T2, class Func> struct Operator : BaseArg {
      template<class T, class Func> using ResultType = Operator<Operator, T, Func>;
      CONSTEXPR Operator(T1 t1, T2 t2, Func func) :func_(func), t1_(t1), t2_(t2) {}
      template<class Arg>
      CONSTEXPR auto operator()(Arg arg) const -> decltype(func_(t1_(arg), t2_(arg))) {
        return func_(t1_(arg), t2_(arg));
      }
    private:
      Func func_;
      T1 t1_;
      T2 t2_;
    };

    template<class Type, class T>struct TypeVal : BaseArg {
      TypeVal(const T Type::*val) :val_(val) {}
      CONSTEXPR T operator()(const Type &type) const { return type.*val_; }
      CONSTEXPR T operator()(const Type *type) const { return type->*val_; }
      CONSTEXPR T operator()(const std::shared_ptr<Type> &type) const { return type.get()->*val_; }
      CONSTEXPR T operator()(const std::unique_ptr<Type> &type) const { return type.get()->*val_; }

      const T Type::*val_;
    };

    struct Arg : BaseArg {
      CONSTEXPR Arg() {}
      template<class T> CONSTEXPR T operator()(const T&t) const { return t; }
      template<class Type, class T>
      CONSTEXPR TypeVal<Type, T> val(const T Type::*val) const {
        return TypeVal<Type, T>(val);
      }
      template<class Type, class T>
      CONSTEXPR TypeVal<Type, T> operator[](const T Type::*val) const {
        return TypeVal<Type, T>(val);
      }

    };

    template<class T>
    struct Data : BaseArg {
      CONSTEXPR Data(T t) :t_(t) {}
      template<class Tmp> T operator()(const Tmp&) const { return t_; }
      T t_;
    };

    template<class Type, class T> struct DataCreater {
      using type = T;
      CONSTEXPR static T Create(T t) { return t; }
    };
    template<class T> struct DataCreater<std::false_type, T> {
      using type = Data<T>;
      CONSTEXPR static Data<T> Create(T t) { return Data<T>(t); }
    };
    template<class T>
    CONSTEXPR typename DataCreater<typename std::is_base_of<BaseArg, T>::type, T>::type CreateData(T t) {
      return DataCreater<std::is_base_of<BaseArg, T>::type, T>::Create(t);
    }

#define MakeMacro(MACRO_NAME) MACRO_NAME(Comparator, ==); \
                              MACRO_NAME(Remainder,  %);  \
                              MACRO_NAME(Adder,      +);  \
                              MACRO_NAME(Subtractor, -);  \
                              MACRO_NAME(Divider,    /);  \
                              MACRO_NAME(Multiplier, *);

#define MakeOperatorUnit(FUNC_NAME, OP)                                                   \
    struct FUNC_NAME {                                                                    \
      CONSTEXPR FUNC_NAME() {}                                                            \
      template<class T1, class T2>                                                        \
      CONSTEXPR auto operator()(const T1 &t1, const T2 &t2) const -> decltype(t1 OP t2) { \
        return t1 OP t2;                                                                  \
      }                                                                                   \
    };
    MakeMacro(MakeOperatorUnit);
#undef MakeOperatorUnit

    template<class T1, class T2, class Op> CONSTEXPR
      Operator<T1, T2, Op> CreateOperator(const T1 &t1, const T2 &t2, Op op) {
      return Operator<T1, T2, Op>(t1, t2, op);
    }
#define MakeOperator(FUNC_NAME, OP)                                                        \
    template<class T1, class T2> CONSTEXPR auto operator OP(const T1 &t1,const T2 &t2)     \
    -> decltype(CreateOperator(CreateData(t1), CreateData(t2), FUNC_NAME())){              \
      return CreateOperator(CreateData(t1), CreateData(t2), FUNC_NAME());                  \
    }
    MakeMacro(MakeOperator);
#undef MakeOperator

#undef MakeMacro

    CONSTEXPR Arg _1 = Arg{};
  }
}

#endif // !INCLUDE_HOLDER_H