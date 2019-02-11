#ifndef FEM_MATH_MATRIX_HPP_
#define FEM_MATH_MATRIX_HPP_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "vector.hpp"
#include "../util/to_string.hpp"

namespace fem {
template <typename _T>
class Matrix {
 public:
  Matrix() : size_(0) {}
  explicit Matrix(unsigned long n)
      : size_(n), row_ptr_(std::vector<unsigned long>(n + 1, 0)) {}
  explicit Matrix(unsigned long n, const _T& v)
      : size_(n), row_ptr_(std::vector<unsigned long>(n + 1, 0)) {
    for (unsigned long i = 0; i < size_; ++i) {
      set(i, i, v);
    }
  }
  Matrix(const Matrix<_T>& copy)
      : size_(copy.size_),
        row_ptr_(copy.row_ptr_),
        col_ind_(copy.col_ind_),
        values_(copy.values_) {}

  inline unsigned long size() const noexcept { return size_; }
  inline unsigned long rows() const noexcept { return size_; }
  inline unsigned long cols() const noexcept { return size_; }
  inline unsigned long max_size() const noexcept { return size_ * size_; }
  inline unsigned long count() const noexcept { return values_.size(); }

  inline unsigned long row_count(const unsigned long& r) const noexcept {
    return row_ptr_[r + 1] - row_ptr_[r];
  }
  inline unsigned long col_row_ind(const unsigned long& r,
                                   const unsigned long& i) const {
    return col_ind_[row_ptr_[r] + i];
  }
  inline _T& at_row_ind(const unsigned long& r, const unsigned long& i) {
    return values_[row_ptr_[r] + i];
  }
  inline _T at_row_ind(const unsigned long& r, const unsigned long& i) const {
    return values_[row_ptr_[r] + i];
  }

  std::string dump(unsigned perc = 5) const {
    std::string dmp;
    for (unsigned long i = 0; i < size_; ++i) {
      for (unsigned long j = 0; j < size_; ++j) {
        dmp += fem::to_string(at(i, j), perc) + " ";
      }
      dmp += "\n";
    }
    return dmp;
  }

  inline void resize(const unsigned long& size) {
    size_ = size;
    row_ptr_ = std::vector<unsigned long>(size_ + 1, 0);
    col_ind_.clear();
    values_.clear();
  }

  template <typename _U>
  inline Matrix<_T>& operator+=(const _U& rhs) {
    *this = *this + rhs;
    return *this;
  }
  template <typename _U>
  inline Matrix<_T>& operator-=(const _U& rhs) {
    *this = *this - rhs;
    return *this;
  }
  template <typename _U>
  inline Matrix<_T>& operator*=(const _U& rhs) {
    *this = *this * rhs;
    return *this;
  }
  template <typename _U>
  inline Matrix<_T>& operator/=(const _U& rhs) {
    *this = *this / rhs;
    return *this;
  }

  _T& operator()(unsigned long r, unsigned long c) {
    if (row_ptr_[r + 1] - row_ptr_[r] == 0) {
      values_.insert(values_.begin() + row_ptr_[r + 1], _T());
      col_ind_.insert(col_ind_.begin() + row_ptr_[r], c);
      for (unsigned long i = r + 1; i < size_ + 1; ++i) {
        row_ptr_[i]++;
      }
      return values_[row_ptr_[r]];
    } else {
      for (unsigned long i = row_ptr_[r]; i < row_ptr_[r + 1]; ++i) {
        if (col_ind_[i] == c) {
          return values_[i];
        }
      }
      values_.insert(values_.begin() + row_ptr_[r + 1], _T());
      col_ind_.insert(col_ind_.begin() + row_ptr_[r + 1], c);
      for (unsigned long i = r + 1; i < size_ + 1; ++i) {
        row_ptr_[i]++;
      }
      return values_[row_ptr_[r + 1] - 1];
    }
  }
  _T operator()(unsigned long r, unsigned long c) const {
    if (row_ptr_[r + 1] - row_ptr_[r] == 0) {
      return _T();
    } else {
      for (std::size_t i = row_ptr_[r]; i < row_ptr_[r + 1]; ++i) {
        if (col_ind_[i] == c) {
          return values_[i];
        }
      }
      return _T();
    }
  }

  _T at(unsigned long r, unsigned long c) const {
    if (row_ptr_[r + 1] - row_ptr_[r] == 0) {
      return _T();
    } else {
      for (std::size_t i = row_ptr_[r]; i < row_ptr_[r + 1]; ++i) {
        if (col_ind_[i] == c) {
          return values_[i];
        }
      }
      return _T();
    }
  }

  void set(unsigned long r, unsigned long c, const _T& val) {
    if (row_ptr_[r + 1] - row_ptr_[r] == 0) {
      if (val != 0) {
        values_.insert(values_.begin() + row_ptr_[r + 1], val);
        col_ind_.insert(col_ind_.begin() + row_ptr_[r], c);
        for (unsigned long i = r + 1; i < size_ + 1; ++i) {
          row_ptr_[i]++;
        }
      }
      return;
    } else {
      for (unsigned long i = row_ptr_[r]; i < row_ptr_[r + 1]; ++i) {
        if (col_ind_[i] == c) {
          values_[i] = val;
          if (val == 0) {
            values_.erase(values_.begin() + i);
            col_ind_.erase(col_ind_.begin() + i);
            for (unsigned long i = r + 1; i < size_ + 1; ++i) {
              row_ptr_[i]--;
            }
          }
          return;
        }
      }
      if (val != 0) {
        values_.insert(values_.begin() + row_ptr_[r + 1], val);
        col_ind_.insert(col_ind_.begin() + row_ptr_[r + 1], c);
        for (unsigned long i = r + 1; i < size_ + 1; ++i) {
          row_ptr_[i]++;
        }
      }
      return;
    }
  }

  void clear() {
    row_ptr_ = std::vector<unsigned long>(size_ + 1, 0);
    col_ind_.clear();
    values_.clear();
  }

  std::vector<unsigned long> row_ptr() const { return row_ptr_; }
  std::vector<unsigned long>* row_ptr_pt() { return &row_ptr_; }
  unsigned long row_ptr(const unsigned long& i) const { return row_ptr_[i]; }
  std::vector<unsigned long> col_ind() const { return col_ind_; }
  std::vector<unsigned long>* col_ind_pt() { return &col_ind_; }
  unsigned long col_ind(const unsigned long& i) const { return col_ind_[i]; }
  std::vector<_T> data() const { return values_; }
  std::vector<_T>* data_pt() { return &values_; }
  _T data(const unsigned long& i) const { return values_[i]; }

 private:
  unsigned long size_ = 0;
  std::vector<unsigned long> row_ptr_;
  std::vector<unsigned long> col_ind_;
  std::vector<_T> values_;
};

template <typename _T>
inline std::ostream& operator<<(std::ostream& out, const Matrix<_T>& rhs) {
  out << '[' << rhs.dump() << ']';
  return out;
}

template <typename _T, typename _U>
inline bool operator==(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  return (lhs.data() == rhs.data() && lhs.col_ind() == rhs.col_ind() &&
          lhs.row_ptr() == rhs.row_ptr());
}
template <typename _T, typename _U>
inline bool operator!=(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  return !(lhs.data() == rhs.data() && lhs.col_ind() == rhs.col_ind() &&
           lhs.row_ptr() == rhs.row_ptr());
}

template <typename _T, typename _U>
inline Matrix<_T> operator+(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  Matrix<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned int r = 0; r < res.size(); ++r) {
    if (lhs.row_count(r) != 0 || rhs.row_count(r) != 0) {
      for (unsigned int c = 0; c < res.size(); ++c) {
        res.set(r, c, lhs.at(r, c) + rhs.at(r, c));
      }
    }
  }
  return res;
}
template <typename _T, typename _U>
inline Matrix<_T> operator-(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  Matrix<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned int r = 0; r < res.size(); ++r) {
    if (lhs.row_count(r) != 0 || rhs.row_count(r) != 0) {
      for (unsigned int c = 0; c < res.size(); ++c) {
        res.set(r, c, lhs.at(r, c) - rhs.at(r, c));
      }
    }
  }
  return res;
}

template <typename _T, typename _U>
inline Matrix<_T> operator*(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  Matrix<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned long r = 0; r < res.size(); ++r) {
    for (unsigned long c = 0; c < res.size(); ++c) {
      _T sum = _T();
      for (unsigned long k = 0; k < res.size(); ++k) {
        sum += (lhs.at(r, k) * rhs.at(k, c));
      }
      res.set(r, c, sum);
    }
  }
  return res;
}
template <typename _T, typename _U>
inline Matrix<_T> operator*(const Matrix<_T>& lhs, const _U& rhs) {
  Matrix<_T> res(lhs);
  for (unsigned long r = 0; r < res.size(); ++r) {
    for (unsigned long c = 0; c < res.size(); ++c) {
      res.set(r, c, res.at(r, c) * rhs);
    }
  }
  return res;
}
template <typename _T, typename _U>
inline Vector<_T> operator*(const Matrix<_T>& lhs, const Vector<_U>& rhs) {
  Vector<_T> res(lhs.size());
  for (unsigned long r = 0; r < lhs.size(); ++r) {
    _T sum = _T();
    for (unsigned long c = 0; c < rhs.size() && c < lhs.size(); ++c) {
      sum += (lhs.at(r, c) * rhs.at(c));
    }
    res.set(r, sum);
  }
  return res;
}
template <typename _T, typename _U>
inline Matrix<_T> operator/(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  Matrix<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned int r = 0; r < res.size(); ++r) {
    if (lhs.row_count(r) != 0 || rhs.row_count(r) != 0) {
      for (unsigned int c = 0; c < res.size(); ++c) {
        res.set(r, c, lhs.at(r, c) / rhs.at(r, c));
      }
    }
  }
  return res;
}

template <typename _T, typename _U>
Matrix<_T> Multiply(const Matrix<_T>& lhs, const Matrix<_U>& rhs) {
  Matrix<_T> res(std::min(lhs.size(), rhs.size()));
  for (unsigned int r = 0; r < res.size(); ++r) {
    if (lhs.row_count(r) != 0 || rhs.row_count(r) != 0) {
      for (unsigned int c = 0; c < res.size(); ++c) {
        res.set(r, c, lhs.at(r, c) * rhs.at(r, c));
      }
    }
  }
  return res;
}

template <typename _T>
Matrix<_T> Transpose(const Matrix<_T>& mat) {
  Matrix<_T> res(mat.size());
  for (unsigned long r = 0; r < res.size(); ++r) {
    if (mat.row_count(r) != 0) {
      for (unsigned long i = mat.row_ptr(r); i < mat.row_ptr(r + 1); ++i) {
        res.set(mat.col_ind(i), r, mat.data(i));
      }
    }
  }
  return res;
}

template <typename _T>
void SaveToFile(const std::string& file_name, const Matrix<_T>& mat) {
  std::ofstream out(file_name.c_str());
  if (out.is_open()) {
    out << mat.size() << '\n';
    for (std::size_t i = 0; i < mat.size(); ++i) {
      out << mat.row_ptr(i + 1) << ' ';
      for (std::size_t j = mat.row_ptr(i); j < mat.row_ptr(i + 1); ++j) {
        out << mat.col_ind(j) << ' ' << fem::to_string(mat.data(j)) << ' ';
      }
      out << '\n';
    }
    out.close();
  }
}

template <typename _T = double>
Matrix<_T> LoadMatFromFile(const std::string& file_name) {
  Matrix<_T> res;
  std::ifstream src(file_name.c_str());
  if (src.is_open()) {
    unsigned long size;
    src >> size;
    res.resize(size);
    std::vector<unsigned long>* row_ptr = res.row_ptr_pt();
    std::vector<unsigned long>* col_ind = res.col_ind_pt();
    std::vector<_T>* data = res.data_pt();
    for (unsigned long i = 0; i < size; ++i) {
      unsigned long row_ind;
      src >> row_ind;
      row_ptr->at(i + 1) = row_ind;
      for (unsigned long j = row_ptr->at(i); j < row_ind; ++j) {
        unsigned long local_col_ind;
        _T val;
        src >> local_col_ind >> val;
        col_ind->push_back(local_col_ind);
        data->push_back(val);
      }
    }
    src.close();
  }
  return res;
}

typedef Matrix<double> Mat;
}  // namespace fem

#endif  // FEM_MATH_MATRIX_HPP_
