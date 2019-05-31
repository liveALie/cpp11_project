#ifndef _C11TEST_TASK_GROUP_HPP_
#define _C11TEST_TASK_GROUP_HPP_

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "any.hpp"
#include "non_copyable.hpp"
#include "variant.hpp"

// TaskGroup的思想是，保存所有Task的future，调用future的get或者wait时，则一起执行，并且时多线程执行。
//所有的run函数就是为了获取每个任务的future，并存储下来。
class TaskGroup : NonCopyable {
  typedef Variant<int, std::string, double, short, unsigned int> RetVariant;

public:
  TaskGroup() {}
  ~TaskGroup() {}
  //返回类型为void
  template <typename R, class = typename std::enable_if<
                            !std::is_same<R, void>::value>::type>
  void Run(Task<R()> &&task) {
    group_.emplace(R(), task.Run());
  }
  //返回类型不为void
  template <typename R, class = typename std::enable_if<
                            !std::is_same<R, void>::value>::type>
  R Run(Task<R()> &task) {
    group_.emplace(R(), task.Run());
  }
  //返回类型为void的
  void Run(Task<void()> &&task) { void_group_.push_back(task.Run()); }

  //这个函数和下一个模板函数是参数解包。
  template <typename F> void Run(F &&f) {
    Run(Task<typename std::result_of<F()>::type()>(std::forward<F>(f)));
  }

  template <typename F, typename... Funs> void Run(F &&f, Funs... funs) {
    Run(std::forward<F>(f));
    Run(std::forward<Funs>(funs)...);
  }

  void Wait() {
    for (auto it = group_.begin(); it != group_.end(); ++it) {
      auto vrt = it->first;
      //访问有返回值类型的future的返回值。
      vrt.Visit([&](int a) { FutureGet<int>(it->second); },
                [&](double a) { FutureGet<double>(it->second); },
                [&](std::string a) { FutureGet<std::string>(it->second); },
                [&](short a) { FutureGet<short>(it->second); },
                [&](unsigned int a) { FutureGet<unsigned int>(it->second); });
    }
    //执行无返回值的任务
    for (auto it = void_group_.begin(); it != void_group_.end(); ++it) {
      it->get();
    }
  }

private:
  template <typename T> void FutureGet(Any &f) {
    f.AnyCast<shared_future<T>>().get();
  }

private:
  multimap<RetVariant, Any> group_;
  vector<std::shared_future<void>> void_group_;
};

//当所有的task执行完。把所有task的future存起来。
// range为容器类型，value_type为task类型，return_type？
template <typename Range>
extern Task<vector<typename Range::value_type::ReturnType>()>
WhenAll(Range &range) {
  typedef typename Range::value_type::ReturnType Return_type;
  auto task = [&range] {
    vector<std::shared_future<Return_type>> fv;
    for (auto &task : range) {
      fv.emplace_back(task.Run());
    }
    vector<Return_type> v;
    for (auto &item : fv) {
      v.emplace_back(item.get());
    }
    return v; //这是返回的task调用get之后的返回值。
  };
  return Task<vector<Return_type>()>(task); //这是whenall的返回值
}
//以下为whenany的实现
template <typename R> struct RangeTrait { typedef R type; };

template <typename R> struct RangeTrait<std::shared_future<R>> {
  typedef R type;
};

template <typename Range>
std::vector<std::shared_future<typename Range::value_type::ReturnType>>
TransForm(Range &range) {
  typedef typename Range::value_type::ReturnType Return_type;
  std::vector<std::shared_future<Return_type>> fv;
  for (auto &task : range) {
    fv.emplace_back(task.Run());
  }
  return fv;
}

template <typename Range>
std::pair<int, typename RangeTrait<typename Range::value_type>::type>
GetAnyResultPair(const Range &fv) {
  size_t size = fv.size();
  while (true) {
    for (size_t i = 0; i < size; ++i) {
      if (fv[i].wait_for(std::chrono::milliseconds(1)) ==
          std::future_status::ready) {
        return std::make_pair(i, fv[i].get());
      }
    }
  }
}

template <typename Range>
extern Task<std::pair<int, typename Range::value_type::ReturnType>()>
WhenAny(Range &range) {
  typedef typename Range::value_type::ReturnType Return_type;
  auto task = [&range] { return GetAnyResultPair(TransForm(range)); };
  return Task<std::pair<int, Return_type>()>(task);
}

//一下为并行算法
template <typename Iterator, typename Function>
void ParallelForeach(Iterator &begin, Iterator &end, Function &func) {
  auto part_num = std::thread::hardware_concurrency();
  auto block_size = std::distance(begin, end) / part_num;
  Iterator last = begin;
  if (block_size > 0) {
    std::advance(last, (part_num - 1) * block_size);
  } else {
    last = end;
    block_size = 1;
  }

  std::vector<std::future<void>> futures;
  for (; begin != last; std::advance(begin, block_size)) {
    futures.emplace_back(std::async([begin, block_size, &func] {
      std::for_each(begin, begin + block_size, func);
    }));
  }

  futures.emplace_back(
      std::async([begin, end, &func] { std::for_each(begin, end, func); }));

  std::for_each(futures.begin(), futures.end(),
                [](std::future<void> &future) { future.wait(); });
}

template <typename... Funs> void ParallelInvoke(Funs &&... funs) {
  TaskGroup group;
  group.Run(std::forward<Funs>(funs)...);
  group.Wait();
}

template <typename Range, typename ReduceFunc>
typename Range::value_type ParallelReduce(Range &range,
                                          typename Range::value_type &init,
                                          ReduceFunc reduce_func) {
  return ParallelReduce<Range, ReduceFunc>(range, init, reduce_func,
                                           reduce_func);
}

template <typename Range, typename RangeFunc, typename ReduceFunc>
typename Range::value_type
ParallelReduce(Range &range, typename Range::value_type &init,
               RangeFunc range_func, ReduceFunc reduce_func) {
  auto part_num = std::thread::hardware_concurrency();
  auto begin = std::begin(range);
  auto end = std::end(range);
  auto block_size = std::distance(begin, end) / part_num;
  typename Range::iterator last = begin;
  if (block_size > 0) {
    std::advance(last, (part_num - 1) * block_size);
  } else {
    last = end;
    block_size = 1;
  }

  typedef typename Range::value_type ValueType;
  std::vector<std::future<ValueType>> futures;
  for (; begin != last; std::advance(begin, block_size)) {
    futures.emplace_back(std::async([begin, &init, block_size, &range_func] {
      return range_func(begin, begin + block_size, init);
    }));
  }

  futures.emplace_back(std::async([begin, &init, block_size, &range_func] {
    return range_func(begin, begin + block_size, init);
  }));
  vector<ValueType> results;
  std::for_each(futures.begin(), futures.end(),
                [&results](std::future<ValueType> &future) {
                  results.emplace_back(future.get());
                });
  return reduce_func(results.begin(), results.end(), init);
}

#endif // _C11TEST_TASK_GROUP_HPP_
