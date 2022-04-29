#pragma once

#include <iostream>
#include <tuple>

using namespace std;

template <std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value>::type
printtp(Tuple &&t) {}

template <std::size_t I = 0, typename Tuple>
    typename std::enable_if <
    I<std::tuple_size<Tuple>::value>::type printtp(Tuple &&t) {
  std::cout << std::get<I>(t) << " ";
  printtp<I + 1>(std::forward<Tuple>(t));
}

template <typename... Args> void Log(Args... args) {
  printtp(std::make_tuple(args...));
  cout << endl;
}
