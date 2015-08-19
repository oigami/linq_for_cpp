#pragma once
#ifndef INCLUDE_RANGE_H
#define INCLUDE_RANGE_H
#include<iostream>
#include<vector>
#include<array>
#include<unordered_map>
#include<initializer_list>
#include<algorithm>

#include<cassert>
#include<linq_for_cpp/lambda.h>

namespace linq {
  namespace detail {
    template <typename R, typename... Args> auto getResultType(R(*)(Args...))->R {}
    template <typename F, typename R, typename... Args> auto getResultType(R(F::*)(Args...))->R {}
    template <typename F, typename R, typename... Args> auto getResultType(R(F::*)(Args...) const)->R {}
    template <typename F, typename R = decltype(getResultType(&F::operator()))> auto getResultType(F)->R;
    template <typename F, typename... Args> struct FuncResult {
      /* If compile error,
         A function is the number of arguments are incorrect  */
      using type = decltype((*static_cast<F*>(0))(Args()...));
    };

    template<class T> class ArrayIterator :public std::iterator<std::forward_iterator_tag, T> {
    public:
      ArrayIterator(const T* it) : it_(it) {};
      ArrayIterator() {};
    public:
      const T &operator*() const { return *it_; }
      ArrayIterator &operator++() { ++it_; return *this; }
      bool operator!=(const ArrayIterator &x) const { return it_ != x.it_; }
      bool operator==(const ArrayIterator &x) const { return it_ == x.it_; }

    private:
      const T* it_;
    };

    template<class T, size_t size_> class ArrayContainer {
    public:
      using value_type = T;
      using const_iterator = typename std::array<T, size_>::const_iterator;
      using const_reverse_iterator = typename std::array<T, size_>::const_reverse_iterator;
      ArrayContainer(const T(&t)[size_]) {
        std::copy(std::begin(t), std::end(t), t_.data());
      }
      size_t size() const { return size_; }
      const_iterator begin() const { return t_.begin(); }
      const_iterator end() const { return t_.end(); }

      const_reverse_iterator rbegin() const { return t_.rbegin(); }
      const_reverse_iterator rend() const { return t_.rend(); }
    private:
      std::array<T, size_> t_;
    };

    template<class T, size_t size_> class ArrayRefContainer {
    public:
      using value_type = T;
      using const_iterator = ArrayIterator<T>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
      ArrayRefContainer(const T(&t)[size_]) :t_(t) {}
      size_t size() const { return size_; }
      const_iterator begin() const { return std::begin(t_); }
      const_iterator end() const { return std::end(t_); }

      const_reverse_iterator rbegin() const { return std::rbegin(t_); }
      const_reverse_iterator rend() const { return std::rend(t_); }
    private:
      const T(&t_)[size_];
    };

    template<class T> class RefContainer {
    public:
      using value_type = typename T::value_type;
      using const_iterator = typename T::const_iterator;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
      RefContainer(const T& t) :t_(t) {}
      size_t size() const { return t_.size(); }
      const_iterator begin() const { return t_.begin(); }
      const_iterator end() const { return t_.end(); }

      const_reverse_iterator rbegin() const { return const_reverse_iterator(t_.end()); }
      const_reverse_iterator rend() const { return const_reverse_iterator(t_.begin()); }
    private:
      const T &t_;
    };

    template<class T> class ReverseContainer {
    public:
      using const_iterator = typename T::const_reverse_iterator;
      using value_type = typename T::value_type;
      ReverseContainer(const T &t) :t_(t) {}
      ReverseContainer(T &&t) :t_(std::move(t)) {}
      size_t size() const { return t_.size(); }
      const_iterator begin() const { return t_.rbegin(); }
      const_iterator end() const { return t_.rend(); }
    private:
      const T t_;
    };

    template<class T> class ReverseRefContainer {
    public:
      using const_iterator = typename T::const_reverse_iterator;
      using value_type = typename T::value_type;
      ReverseRefContainer(const T &t) :t_(t) {}
      size_t size() const { return t_.size(); }
      const_iterator begin() const { return t_.rbegin(); }
      const_iterator end() const { return t_.rend(); }
    private:
      const T &t_;
    };


    template<class Child, class TRange> class IEnumerableLinq;
    template<class Child, class TRange> class ToContainer;

    template<class TRange, class Func> class CSelect :public IEnumerableLinq<CSelect<TRange, Func>, TRange> {
    public:
      using MyType = CSelect<TRange, Func>;
      using value_type = typename FuncResult<Func, typename TRange::value_type>::type;
      class iterator {
        using i_const_iterator = typename TRange::const_iterator;
        Func func_;
        i_const_iterator t_;
      public:
        iterator(i_const_iterator it, Func func) : func_(func), t_(it) {}
        value_type operator*() const { return func_(*t_); }
        iterator &operator++() { ++t_; return *this; }
        bool operator!=(const iterator &x) const { return t_ != x.t_; }
      };
      using const_iterator = iterator;
      CSelect(const TRange &range, Func func) : range_(range), func_(func) {}
      iterator begin() { return iterator(range_.begin(), func_); }
      iterator end() { return iterator(range_.end(), func_); }

    private:
      void operator=(const CSelect&) = delete;
      TRange range_;
      Func func_;
    };

    template<class TRange, class Func> class SelectMany :public IEnumerableLinq<SelectMany<TRange, Func>, TRange> {
    public:
      using value_type = typename FuncResult<Func, typename TRange::value_type>::type;
      SelectMany(const TRange &range, Func func) : range_(range), func_(func) {}

    private:
      void operator=(const SelectMany<TRange, Func>&) = delete;

      TRange range_;
      Func func_;
    };

    template<class TRange, class Func> class CWhere : public IEnumerableLinq<CWhere<TRange, Func>, TRange> {
    public:
      using value_type = typename TRange::value_type;
      class iterator {
        using i_const_iterator = typename TRange::const_iterator;
        i_const_iterator t_;
        i_const_iterator end_;
        Func func_;
      public:
        iterator(Func func, i_const_iterator it, i_const_iterator end) : t_(it), end_(end), func_(func) {}
        iterator(Func func, i_const_iterator end) : t_(end), end_(end), func_(func) {}
        auto operator*() const -> decltype(*t_) { return *t_; }
        iterator &operator++() {
          while (++t_ != end_) {
            if (func_(*t_)) return *this;
          }
          return *this;
        }
        bool operator!=(const iterator &x) const { return t_ != x.t_; }
      };
      using const_iterator = iterator;
      CWhere(const TRange &range, Func func) : range_(range), func_(func) {}

      iterator begin() { return iterator(func_, range_.begin(), range_.end()); }
      iterator end() { return iterator(func_, range_.end()); }
    private:
      void operator=(const CWhere&) = delete;

      TRange range_;
      Func func_;
    };

    template<class TRange> class Skip : public IEnumerableLinq<Skip<TRange>, TRange> {
    public:
      using value_type = typename TRange::value_type;
      class iterator {
        using i_const_iterator = typename TRange::const_iterator;
        i_const_iterator t_;
      public:
        iterator(i_const_iterator it) : t_(it) {}
        value_type operator*() const { return *t_; }
        iterator &operator++() { ++t_; return *this; }
        bool operator!=(const iterator &x) const { return t_ != x.t_; }
      };
      Skip(const TRange &range, size_t count) : range_(range), count_(count) {}

      using const_iterator = iterator;

      iterator begin()const {
        auto it = range_.begin(), end = range_.end();
        size_t count = 0;
        while (count++ < count_) {
          if (it != end) break;
          ++it;
        }
        return iterator(it);
      }
      iterator end() const { return iterator(range_.end()); }
    private:
      void operator=(const Skip&) = delete;

      size_t count_;
      TRange range_;
    };

    template<class TRange> class Take : public IEnumerableLinq<Take<TRange>, TRange> {
    public:
      using value_type = typename TRange::value_type;
      class iterator {
        using i_const_iterator = typename TRange::const_iterator;
        i_const_iterator t_;
        size_t current_;
      public:
        iterator(i_const_iterator it, size_t current) : t_(it), current_(current) {}
        value_type operator*() const { return *t_; }
        iterator &operator++() { ++t_; ++current_; return *this; }
        bool operator!=(const iterator &x) const { return t_ != x.t_ && current_ != x.current_; }
      };
      using const_iterator = iterator;
      Take(const TRange &range, size_t count) : range_(range), count_(count) {}

      iterator begin() const { return iterator(range_.begin(), 0u); }
      iterator end() const { return iterator(range_.end(), count_); }
    private:
      void operator=(const Take&) = delete;

      TRange range_;
      size_t count_;
    };

    template<class TRange, class Func> class TakeWhile : public IEnumerableLinq<TakeWhile<TRange, Func>, TRange> {
    public:
      using value_type = typename TRange::value_type;
      class iterator {
        using i_const_iterator = typename TRange::const_iterator;
        i_const_iterator t_;
        Func func_;
      public:
        iterator(i_const_iterator it, Func func) : t_(it), func_(func) {}
        value_type operator*() const { return *t_; }
        iterator &operator++() { ++t_; return *this; }
        bool operator!=(const iterator &x) const { return t_ != x.t_ && (func_(*t_)); }
      };
      using const_iterator = iterator;
      TakeWhile(const TRange &range, Func func) : range_(range), func_(func) {}

      iterator begin() const { return iterator(range_.begin(), func_); }
      iterator end() const { return iterator(range_.end(), func_); }
    private:
      void operator=(const TakeWhile &) = delete;

      TRange range_;
      Func func_;
    };

    template<class Child> class ChildChecker {
    protected:
      Child &GetChild() {
        static_assert(std::is_base_of<ChildChecker, Child>::value, "does not inherit an A to B");
        assert(dynamic_cast<Child*>(this));
        return static_cast<Child&>(*this);
      }
#ifndef NDEBUG
      virtual void debug_virtual_func() {};
#endif
    };

    template<class Child, class TRange> class ToContainer : public ChildChecker<Child> {
    public:
      using value_type = typename TRange::value_type;
      std::vector<value_type> ToVector() {
        std::vector<value_type> ret;
        for (const auto& i : this->GetChild()) {
          ret.push_back(i);
        }
        return ret;
      }

      template<class Func> void ForEach(const Func &func) {
        for (const auto&& i : this->GetChild()) {
          func(i);
        }
      }

      value_type Sum() {
        value_type t{};
        for (const auto&& i : this->GetChild()) {
          t += i;
        }
        return t;
      }

      template<class T, class Func> T Aggregate(const T &t, Func func) {
        Child &child = this->GetChild();
        T working = t;
        for (const auto&& i : child) {
          working = func(working, i);
        }
        return working;
      }

      template<class Func> value_type Aggregate(Func func) {
        Child &child = this->GetChild();
        auto it = child.begin(), end = child.end();
        if (!(it != end)) throw std::exception("empty");
        value_type working = *it;
        while (++it != end) working = func(working, *it);
        return working;
      }

      value_type Max() {
        return Aggregate([](value_type work, value_type now) { return std::max(work, now); });
      }

      value_type Min() {
        return Aggregate([](value_type work, value_type now) { return std::min(work, now); });
      }

      /*value_type Average() {
        size_t cnt = 0;
        value_type sum = Aggregate([&](value_type work, value_type now) { ++cnt; return work + now; });
        return sum / cnt;
      }*/

      size_t Count() {
        return Aggregate(0, [](int work, value_type now) { return ++work; });
      }

      template<class Func> bool All(Func func) {
        for (const auto&& i : this->GetChild()) {
          if (!func(i)) return false;
        }
        return true;
      }

      template<class Func> bool Any(Func func) {
        for (const auto&& i : this->GetChild()) {
          if (func(i)) return true;
        }
        return false;
      }

      value_type First() {
        for (const auto&& i : this->GetChild()) {
          return i;
        }
        throw std::exception("Empty");
      }

      value_type FirstOrDefault(const value_type &value) {
        for (const auto&& i : this->GetChild()) {
          return i;
        }
        return value;
      }

      value_type Single() {
        Child &child = this->GetChild();
        auto it = child.begin(), end = child.end();
        if (!(it != end)) throw std::exception("empty");
        auto&& ret = *it;
        if (++it != end) throw std::exception("Multiple");
        return ret;
      }

      value_type SingleOrDefault(const value_type &value) {
        Child &child = this->GetChild();
        auto it = child.begin(), end = child.end();
        if (!(it != end)) return value;
        auto&& ret = *it;
        if (++it != end) throw std::exception("Multiple");
        return ret;
      }

      bool Contains(const value_type &value) {
        for (const auto&& i : this->GetChild()) {
          if (value == i) return true;
        }
        return false;
      }

      /// <summary>
      /// óvëfÇ™ä‹Ç‹ÇÍÇƒÇ¢ÇÈÇ©Çîªíf
      /// </summary>
      /// <param name="value">åüçıÇ∑ÇÈíl</param>
      /// <param name="comparer">ílÇî‰ärÇ∑ÇÈä÷êî</param>
      /// <returns></returns>
      template<class Func> bool Contains(const value_type &value, Func comparer) {
        for (const auto&& i : this->GetChild()) {
          if (comparer(value, i)) return true;
        }
        return false;
      }

      template<class Func_Arg1>
      auto GroupBy(Func_Arg1 func) -> std::unordered_map<typename FuncResult<Func_Arg1, value_type>::type, std::vector<value_type>> {
        Child &child = this->GetChild();
        std::unordered_map<typename FuncResult<Func_Arg1, value_type>::type, std::vector<value_type>> ret;
        for (const auto&& i : this->GetChild()) {
          ret[func(i)].push_back(i);
        }
        return ret;
      }
#if 0
      template<class Func_Arg1, class = void> auto GroupBy(Func_Arg1 func) -> int {
        static_assert(0, "A function is the number of arguments are incorrect");
      }
#endif // 0
    };

    template<class Child, class TRange>
    class IEnumerableLinq : public ToContainer<Child, TRange> {
    public:
      using value_type = typename TRange::value_type;

      template<class Func> detail::CSelect<Child, Func> Select(const Func &func) {
        return detail::CSelect<Child, Func>(this->GetChild(), func);
      }

      template<class Func> detail::SelectMany<Child, Func> SelectMany(const Func &func) {
        return detail::SelectMany<Child, Func>(this->GetChild(), func);
      }

      template<class Func> detail::CWhere<Child, Func> Where(const Func &func) {
        return detail::CWhere<Child, Func>(this->GetChild(), func);
      }

      detail::Skip<Child> Skip(size_t count) {
        return detail::Skip<Child>(this->GetChild(), count);
      }

      detail::Take<Child> Take(size_t count) {
        return detail::Take<Child>(this->GetChild(), count);
      }

      template<class Func> detail::TakeWhile<Child, Func> TakeWhile(Func func) {
        return detail::TakeWhile<Child, Func>(this->GetChild(), func);
      }

    protected:
      void operator=(const IEnumerableLinq&) = delete;
      IEnumerableLinq() {}
    };

    class IncrementIterator : public std::iterator<std::forward_iterator_tag, int> {
    public:
      IncrementIterator(int counter, int add_count = 1) : counter_(counter), add_count_(add_count) {};
      IncrementIterator() {};
    public:
      const int &operator*() const { return counter_; }
      IncrementIterator &operator++() { counter_ += add_count_; return *this; }
      bool operator!=(const IncrementIterator &x) const { return counter_ < x.counter_; }
      bool operator==(const IncrementIterator &x) const { return counter_ == x.counter_; }
      bool operator<(const IncrementIterator &x) const { return counter_ < x.counter_; }
      bool operator<=(const IncrementIterator &x) const { return counter_ <= x.counter_; }

    private:
      int counter_, add_count_;
    };

    class Range {
    public:
      using value_type = int;
      using const_iterator = IncrementIterator;
      Range(int start, int count) :start_(start), count_(count) {}
      IncrementIterator begin() const { return IncrementIterator(start_); }
      IncrementIterator end() const { return IncrementIterator(start_ + count_); }
      size_t size() const { return count_ - start_; }

    private:
      int start_, count_;
    };

  }

  template<class TContainer> class IEnumerable :
    public detail::IEnumerableLinq<IEnumerable<TContainer>, TContainer> {
  public:
    using const_iterator = typename TContainer::const_iterator;
    IEnumerable(const TContainer &container) : container_(container) {}
    IEnumerable(TContainer &&container) : container_(std::move(container)) {}
    size_t Count() const { return container.size(); }

    const_iterator begin() const { return container_.begin(); }
    const_iterator end() const { return container_.end(); }
  private:
    TContainer container_;
  };

  struct Enumerable {
    static IEnumerable<detail::Range> Range(int start, int count) {
      return IEnumerable<detail::Range>(detail::Range(start, count));
    }

    template<class T> static IEnumerable<T> From(const T &t) {
      return IEnumerable<T>(t);
    }

    template<class T> static IEnumerable<T> From(T &&t) {
      return IEnumerable<T>(std::move(t));
    }

  };

  IEnumerable<detail::Range> Range(int start, int count) {
    return Enumerable::Range(start, count);
  }

  template<class T>
  IEnumerable<detail::RefContainer<std::initializer_list<T>>> From(const std::initializer_list<T> &t) {
    return Enumerable::From(detail::RefContainer<std::initializer_list<T>>(t));
  }
  template<class T> IEnumerable<std::initializer_list<T>> From(std::initializer_list<T> &&t) {
    return Enumerable::From(std::move(t));
  }
  template<class T> IEnumerable<detail::RefContainer<T>> From(T &t) {
    return Enumerable::From(detail::RefContainer<T>(t));
  }
  template<class T> IEnumerable<detail::RefContainer<T>> From(const T &t) {
    return Enumerable::From(detail::RefContainer<T>(t));
  }
  template<class T> IEnumerable<T> From(T &&t) {
    return Enumerable::From(std::move(t));
  }
  template<class T, size_t size>
  IEnumerable<detail::ArrayRefContainer<T, size>> From(const T(&t)[size]) {
    return Enumerable::From(detail::ArrayRefContainer<T, size>(t));
  }
  template<class T, size_t size>
  IEnumerable<detail::ArrayRefContainer<T, size>> From(T(&t)[size]) {
    return Enumerable::From(detail::ArrayRefContainer<T, size>(t));
  }

  template<class T> IEnumerable<T> FromCopy(const T &t) {
    return Enumerable::From(t);
  }
  template<class T, size_t size> IEnumerable<detail::ArrayContainer<T, size>> FromCopy(const T(&t)[size]) {
    return Enumerable::From(detail::ArrayContainer<T, size>(t));
  }

  template<class T>
  IEnumerable<detail::ReverseRefContainer<std::initializer_list<T>>> FromReverse(const std::initializer_list<T> &t) {
    return FromReverse(t);
  }
  template<class T>
  IEnumerable<detail::ReverseContainer<std::initializer_list<T>>> FromReverse(std::initializer_list<T> &&t) {
    return FromReverse(std::move(t));
  }
  template<class T> IEnumerable<detail::ReverseRefContainer<T>> FromReverse(const T &t) {
    return Enumerable::From(detail::ReverseRefContainer<T>(t));
  }
  template<class T> IEnumerable<detail::ReverseContainer<T>> FromReverse(T &&t) {
    return Enumerable::From(detail::ReverseContainer<T>(std::move(t)));
  }
  template<class T, size_t size>
  IEnumerable<detail::ReverseContainer<detail::ArrayRefContainer<T, size>>> FromReverse(const T(&t)[size]) {
    using Container = detail::ArrayRefContainer<T, size>;
    return Enumerable::From(detail::ReverseContainer<Container>(Container(t)));
  }
  template<class T, size_t size>
  IEnumerable<detail::ReverseContainer<detail::ArrayRefContainer<T, size>>> FromReverse(T(&t)[size]) {
    const T(&tt)[size] = t;
    return FromReverse(tt);
  }
}

#endif // !INCLUDE_RANGE_H
