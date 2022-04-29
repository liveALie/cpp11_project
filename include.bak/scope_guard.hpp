#pragma once

template <typename F> class ScopeGuard {
public:
  explicit ScopeGuard(F &&f) : fun_(std::move(f)), dismiss_(false) {}
  explicit ScopeGuard(const F &f) : fun_(f), dismiss_(false) {}

  ~ScopeGuard() {
    if (!dismiss_ && fun_) {
      fun_();
    }
  }

  ScopeGuard(ScopeGuard &&rhs)
      : fun_(std::move(rhs.fun_)), dismiss_(rhs.dismiss_) {
    rhs.Dismiss();
  }
  void Dismiss() { dismiss_ = true; }

private:
  F fun_;
  bool dismiss_;

  ScopeGuard();
  ScopeGuard(const ScopeGuard &);
  ScopeGuard &operator=(const ScopeGuard &);
};