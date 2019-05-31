#pragma once

#include "visitor.hpp"
#include <iostream>

struct SubA;
struct SubB;

struct Base {
public:
  using MyVisitor = Visitor<SubA, SubB>;
  virtual void Accept(MyVisitor &) = 0;
};

struct SubA : Base {
  double val;
  void Accept(Base::MyVisitor &v) { v.Visit(*this); }
};

struct SubB : Base {
  double val;
  void Accept(Base::MyVisitor &v) { v.Visit(*this); }
};

struct PrintVisitor : Base::MyVisitor {
  void Visit(const SubA &a) { std::cout << "from subA" << a.val << std::endl; }

  void Visit(const SubB &b) { std::cout << "from subB" << b.val << std::endl; }
};