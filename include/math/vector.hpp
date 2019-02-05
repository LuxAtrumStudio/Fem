#ifndef FEM_MATH_VECTOR_HPP_
#define FEM_MATH_VECTOR_HPP_

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <string>

namespace fem {
template <typename _T>
class Vector {
 public:
  Vector() : size_(0), data_(nullptr) {}
  explicit Vector(const unsigned long& n) : size_(n) {
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::fill(data_, data_ + size_, _T());
  }
  explicit Vector(const unsigned long& n, const _T& v) : size_(n) {
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::fill(data_, data_ + size_, v);
  }
  explicit Vector(const std::initializer_list<_T>& args) : size_(args.size()) {
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::copy(args.begin(), args.end(), data_);
  }
  Vector(const Vector<_T>& copy) : size_(copy.size_) {
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::copy(copy.begin(), copy.end(), data_);
  }
  ~Vector() { free(data_); }

  Vector<_T>& operator=(const std::initializer_list<_T>& args) {
    free(data_);
    size_ = args.size();
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::copy(args.begin(), args.end(), data_);
  }
  Vector<_T>& operator=(const Vector<_T>& copy) {
    free(data_);
    size_ = copy.size();
    data_ = (_T*)std::malloc(size_ * sizeof(_T));
    std::copy(copy.begin(), copy.end(), data_);
  }

  inline _T& operator[](const unsigned long& i) { return data_[i]; }
  inline const _T& operator[](const unsigned long& i) const { return data_[i]; }

  inline unsigned long size() const noexcept { return size_; }
  std::string dump() const {
    std::string dmp;
    for (unsigned long i = 0; i < size_; ++i) {
      dmp += std::to_string(data_[i]) + ' ';
    }
    return dmp;
  }

  template <typename _U>
  inline typename std::enable_if<std::is_arithmetic<_U>::value>::type fill(
      const _U& v) {
    std::fill(data_, data_ + size_, static_cast<_T>(v));
  }
  template <typename _U>
  inline void fill(_U (*func)(const unsigned long&)) {
    for (unsigned int i = 0; i < size_; ++i) {
      data_[i] = static_cast<_T>(func(i));
    }
  }
  template <typename _U>
  inline void fill(_U (*func)()) {
    for (unsigned int i = 0; i < size_; ++i) {
      data_[i] = static_cast<_T>(func());
    }
  }
  inline void clear() { std::fill(data_, data_ + size_, _T()); }

  inline _T* begin() { return data_; }
  inline const _T* begin() const { return data_; }
  inline const _T* cbegin() const { return data_; }
  inline _T* end() { return data_ + size_; }
  inline const _T* end() const { return data_ + size_; }
  inline const _T* cend() const { return data_ + size_; }

  template <typename _U>
  inline Vector<_T>& operator+=(const Vector<_U>& rhs) {
    for (unsigned long i = 0; i < std::min(size_, rhs.size()); ++i) {
      data_[i] += rhs[i];
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator-=(const Vector<_U>& rhs) {
    for (unsigned long i = 0; i < std::min(size_, rhs.size()); ++i) {
      data_[i] -= rhs[i];
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator*=(const Vector<_U>& rhs) {
    for (unsigned long i = 0; i < std::min(size_, rhs.size()); ++i) {
      data_[i] *= rhs[i];
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator/=(const Vector<_U>& rhs) {
    for (unsigned long i = 0; i < std::min(size_, rhs.size()); ++i) {
      data_[i] /= rhs[i];
    }
    return *this;
  }

  template <typename _U>
  inline Vector<_T>& operator+=(const _U& rhs) {
    for (unsigned long i = 0; i < size_; ++i) {
      data_[i] += rhs;
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator-=(const _U& rhs) {
    for (unsigned long i = 0; i < size_; ++i) {
      data_[i] -= rhs;
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator*=(const _U& rhs) {
    for (unsigned long i = 0; i < size_; ++i) {
      data_[i] *= rhs;
    }
    return *this;
  }
  template <typename _U>
  inline Vector<_T>& operator/=(const _U& rhs) {
    for (unsigned long i = 0; i < size_; ++i) {
      data_[i] /= rhs;
    }
    return *this;
  }

 private:
  unsigned long size_ = 0;
  _T* data_ = nullptr;
};

template <typename _T>
inline std::ostream& operator<<(std::ostream& out, const Vector<_T>& rhs) {
  out << '<' << rhs.dump() << '>';
  return out;
}

template <typename _T, typename _U>
inline bool operator==(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename _T, typename _U>
inline bool operator!=(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename _T, typename _U>
inline Vector<_T> operator+(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  Vector<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] = lhs[i] + rhs[i];
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator-(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  Vector<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] = lhs[i] - rhs[i];
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator*(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  Vector<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] = lhs[i] * rhs[i];
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator/(const Vector<_T>& lhs, const Vector<_U>& rhs) {
  Vector<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] = lhs[i] / rhs[i];
  }
  return res;
}

template <typename _T, typename _U>
inline Vector<_T> operator+(const Vector<_T>& lhs, const _U& rhs) {
  Vector<_T> res(lhs);
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] += rhs;
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator-(const Vector<_T>& lhs, const _U& rhs) {
  Vector<_T> res(lhs);
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] -= rhs;
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator*(const Vector<_T>& lhs, const _U& rhs) {
  Vector<_T> res(lhs);
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] *= rhs;
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator/(const Vector<_T>& lhs, const _U& rhs) {
  Vector<_T> res(lhs);
  for (unsigned long i = 0; i < res.size(); ++i) {
    res[i] /= rhs;
  }
  return res;
}

typedef Vector<double> Vec;

}  // namespace fem

#endif  // FEM_MATH_VECTOR_HPP_