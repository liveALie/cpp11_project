#ifndef _C11TEST_LIST_NODE_HPP_
#define _C11TEST_LIST_NODE_HPP_

typedef int Rank;
//#define ListNodePosi ListNode<T>*
////列表节点位置,这个地方为什么不用typedef，就是宏定义与typedef的区别。

template <typename T> struct ListNode {
  typedef ListNode<T> *ListNodePosi;
  T data;
  ListNodePosi pred;
  ListNodePosi succ;
  ListNode() //针对header和tailer
  {}

  ListNode(T e, ListNodePosi p = NULL, ListNodePosi s = NULL)
      : data(e), pred(p), succ(s) {}

  ListNodePosi insertAsPred(const T &e) {
    ListNodePosi p = new ListNode<T>(e, pred, this);
    pred->succ = p;
    pred = p;
    return p;
  }

  ListNodePosi insertAsSucc(const T &e) {
    ListNodePosi p = new ListNode<T>(e, this, succ);
    succ->pred = p;
    succ = p;
    return p;
  }
};

#endif // _C11TEST_LIST_NODE_HPP_
