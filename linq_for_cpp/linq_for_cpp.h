#pragma once
#ifndef INCLUDE_RANGE_H
#define INCLUDE_RANGE_H
#include<iterator>
#include<iostream>
#include<vector>
#include<unordered_map>
#include<algorithm>

#include<cassert>
#include<linq_for_cpp/holder.h>

namespace linq {
  template<class TContainer> class IEnumerable;
  namespace detail {
    template <typename R, typename... Args> auto getResultType(R(*)(Args...))->R {}
    template <typename F, typename R, typename... Args> auto getResultType(R(F::*)(Args...))->R {}
    template <typename F, typename R, typename... Args> auto getResultType(R(F::*)(Args...) const)->R {}
    template <typename F, typename R = decltype(getResultType(&F::operator()))> auto getResultType(F)->R;
    template <typename F, typename... Args> struct FuncResult {
      /* If compile error,
      /* A function is the number of arguments are incorrect  */
      using type = decltype((*(F*)0)(Args()...));
    };

    template<class T, size_t size_> class ArrayContainer {
    public:
      using value_type = T;
      using const_iterator = const T*;
      ArrayContainer(const T(&t)[size_]) {
        begin_ = std::begin(t);
        end_ = std::end(t);
      }
      size_t size() const { return size_; }

      const_iterator begin()const { return begin_; }
      const_iterator end()const { return end_; }
      const_iterator begin_;
      const_iterator end_;
    };

    template<class TContainer> class IEnumerableIterator {
    public:
      using IteratorType = typename TContainer::const_iterator;
      using value_type = typename TContainer::value_type;
      IEnumerableIterator(const TContainer &t) :begin_(t.begin()), end_(t.end()), it_(t.begin()), current_(t.begin()) {}
      void reset(const TContainer &t) {
        begin_ = t.begin();
        end_ = t.end();
        it_ = begin_;
        current_ = begin_;
      }
      bool next() {
        if (it_ == end_) return false;
        current_ = it_;
        ++it_;
        return true;
      }
      size_t size() const { return size_; }
      const value_type &front() const { return *current_; }

      IteratorType begin() const { return begin_; }
      IteratorType end() const { return end_; }
    private:
      IteratorType begin_;
      IteratorType end_;
      IteratorType current_;
      IteratorType it_;
    };

    template<class Child, class TRange> class IEnumerableLinq;
    template<class Child, class TRange> class ToContainer;

    template<class TRange, class Func> class Select :public IEnumerableLinq<Select<TRange, Func>, TRange> {
    public:
      friend ToContainer<Select<TRange, Func>, TRange>;
      using MyType = Select<TRange, Func>;
      using value_type = typename FuncResult<Func, typename TRange::value_type>::type;
      Select(const TRange &range, Func func) : range_(range), func_(func) {}

    protected:
      bool next() { return range_.next(); }
      value_type front() { return func_(range_.front()); }

    private:
      void operator=(const Select&) = delete;

      TRange range_;
      Func func_;
    };

    template<class TRange, class Func> class SelectMany :public IEnumerableLinq<Select<TRange, Func>, TRange> {
    public:
      friend ToContainer<Select<TRange, Func>, TRange>;
      using value_type = typename FuncResult<Func, typename TRange::value_type>::type;
      SelectMany(const TRange &range, Func func) : range_(range), func_(func) {}

    protected:
      bool next() { return range_.next(); }
      value_type front() { return func_(range_.front()); }

    private:
      void operator=(const Select&) = delete;

      TRange range_;
      Func func_;
    };

    template<class TRange, class Func> class Where : public IEnumerableLinq<Where<TRange, Func>, TRange> {
    public:
      using value_type = typename TRange::value_type;
      Where(const TRange &range, Func func) : range_(range), func_(func) {}
      bool next() {
        while (range_.next()) {
          if (func_(range_.front()))
            return true;
        }
        return false;
      }
      value_type front() { return range_.front(); }
    private:
      void operator=(const Where&) = delete;

      TRange range_;
      Func func_;
    };

    template<class TRange> class Skip : public IEnumerableLinq<Skip<TRange>, TRange> {
      using value_type = typename TRange::value_type;
      Skip(const TRange &range, size_t count) : range_(range), count_(count), is_skiped_(false) {}
      bool next() {
        if (!is_skiped_) {
          is_skiped_ = true;
          size_t count = 0;
          while (count++ < count_) {
            if (!range_.next())return false;
          }
        }
        return range_.next();
      }
      value_type front() { return range_.front(); }
    private:
      void operator=(const Skip&) = delete;

      size_t count_;
      TRange range_;
      bool is_skiped_;
    };

    template<class TRange> class Take : public IEnumerableLinq<Take<TRange>, TRange> {
      using value_type = typename TRange::value_type;
      Take(const TRange &range, size_t count) : range_(range), count_(count), current_(0) {}
      bool next() {
        if (current_++ < count_)return false;
        return range_.next();
      }
      value_type front() { return range_.front(); }
    private:
      void operator=(const Take&) = delete;

      size_t count_;
      size_t current_;
      TRange range_;
    };

    template<class TRange, class Func> class TakeWhile : public IEnumerableLinq<Take<TRange>, TRange> {
      using value_type = typename TRange::value_type;
      TakeWhile(const TRange &range, Func func) : range_(range), count_(count), current_(0) {}
      bool next() {
        if (!range_.next())return false;
        return func_(range_.front());
      }
      value_type front() { return range_.front(); }
    private:
      void operator=(const Take&) = delete;

      Func func_;
      size_t current_;
      TRange range_;
    };

    template<class Child> class ChildChecker {
    protected:
      Child &GetChild() {
        static_assert(std::is_base_of<ChildChecker, Child>::value, "does not inherit an A to B");
        assert(dynamic_cast<Child*>(this));
        return static_cast<Child&>(*this);
      }
#ifndef NDEBUG
      virtual void func() {};
#endif
    };

    template<class Child, class TRange> class ToContainer : public ChildChecker<Child> {
    public:
      using value_type = typename TRange::value_type;
      std::vector<value_type> ToVector() {
        std::vector<value_type> ret;
        Child &child = this->GetChild();
        while (child.next()) {
          ret.push_back(child.front());
        }
        return ret;
      }

      template<class Func> void ForEach(const Func &func) {
        Child &child = this->GetChild();
        while (child.next()) {
          func(child.front());
        }
      }

      value_type Sum() {
        value_type t = 0;
        Child &child = this->GetChild();
        while (child.next()) {
          t += child.front();
        }
        return t;
      }

      template<class T, class Func> T Aggregate(const T &t, Func func) {
        Child &child = this->GetChild();
        T working = t;
        while (child.next()) {
          working = func(working, child.front());
        }
        return working;
      }

      template<class Func> value_type Aggregate(Func func) {
        Child &child = this->GetChild();
        if (!child.next())throw std::exception("0 element");
        value_type working = child.front();
        while (child.next()) {
          working = func(working, child.front());
        }
        return working;
      }

      value_type Max() {
        return Aggregate([](value_type work, value_type now) { return std::max(work, now); });
      }

      value_type Min() {
        return Aggregate([](value_type work, value_type now) {return std::min(work, now); });
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
        Child &child = this->GetChild();
        while (child.next()) {
          if (!func(child.front()))return false;
        }
        return true;
      }

      template<class Func> bool Any(Func func) {
        Child &child = this->GetChild();
        while (child.next()) {
          if (func(child.front()))return true;
        }
        return false;
      }

      value_type First() {
        Child &child = this->GetChild();
        if (!child.next()) throw std::exception("Empty");
        return child.front();
      }

      value_type FirstOrDefault(const value_type &value) {
        Child &child = this->GetChild();
        if (!child.next()) return value;
        return child.front();
      }

      value_type Single() {
        Child &child = this->GetChild();
        if (!child.next()) std::exception("empty");
        value_type ret = child.front();
        if (child.next()) std::exception("Multiple");
        return ret;
      }

      value_type SingleOrDefault(const value_type &value) {
        Child &child = this->GetChild();
        if (!child.next()) return value;
        value_type ret = child.front();
        if (child.next()) std::exception("Multiple");
        return ret;
      }

      bool Contains(const value_type &value) {
        Child &child = this->GetChild();
        while (child.next()) {
          if (value == child.front()) return true;
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
        Child &child = this->GetChild();
        while (child.next()) {
          if (comparer(value, child.front())) return true;
        }
        return false;
      }

      template<class Func_Arg1>
      auto GroupBy(Func_Arg1 func) -> std::unordered_map<typename FuncResult<Func_Arg1,value_type>::type, std::vector<value_type>> {
        Child &child = this->GetChild();
        std::unordered_map<typename FuncResult<Func_Arg1, value_type>::type, std::vector<value_type>> ret;
        while (child.next()) {
          auto val = child.front();
          ret[func(val)].push_back(val);
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
      template<class T>using Result = typename FuncResult<T, value_type>::type;
      template<class Func> auto Select(const Func &func) -> detail::Select<Child, Func> {
        return detail::Select<Child, Func>(this->GetChild(), func);
      }

      template<class Func> detail::Select<Child, Func> SelectMany(const Func &func) {
        return detail::Select<Child, Func>(this->GetChild(), func);
      }

      template<class Func> detail::Where<Child, Func> Where(const Func &func) {
        return detail::Where<Child, Func>(this->GetChild(), func);
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
      // IncrementIterator() : add_count_(counter_) {};
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
      friend IEnumerable<Range>;
    protected:
      Range() {}
    private:
      int start_, count_;
    };

  }

  template<class TContainer> class IEnumerable :
    public detail::IEnumerableLinq<IEnumerable<TContainer>, TContainer>,
    public detail::IEnumerableIterator<TContainer> {
  public:
    using Iterator = detail::IEnumerableIterator<TContainer>;
    using value_type = typename TContainer::value_type;
    using const_iterator = typename Iterator::IteratorType;

    IEnumerable(const TContainer &t) : Iterator(t), size_(t.size()) {}
    size_t Count() const { return size_; }
  private:
    size_t size_;
    void operator=(const IEnumerable&) = delete;
  };

  struct Enumerable {
    static IEnumerable<detail::Range> Range(int start, int count) {
      return IEnumerable<detail::Range>(detail::Range(start, count));
    }
    template<class T> static IEnumerable<T> From(const T &t) {
      return IEnumerable<T>(t);
    }
    template<class T, size_t size> static IEnumerable<detail::ArrayContainer<T, size>> From(const T(&t)[size]) {
      return IEnumerable<detail::ArrayContainer<T, size>>(detail::ArrayContainer<T, size>{t});
    }
  };

  IEnumerable<detail::Range> Range(int start, int count) {
    return Enumerable::Range(start, count);
  }

  template<class T> IEnumerable<T> From(const T &t) {
    return Enumerable::From(t);
  }

  template<class T, size_t size>
  IEnumerable<detail::ArrayContainer<T, size>> From(const T(&t)[size]) {
    return Enumerable::From(t);
  }
}

#endif // !INCLUDE_RANGE_H
