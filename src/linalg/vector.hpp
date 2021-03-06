#ifndef ARTA_MATH_VECTOR_HPP_
#define ARTA_MATH_VECTOR_HPP_

#include <string>
#include <vector>

namespace arta {
namespace linalg {
  class Vector {
   public:
    Vector();
    explicit Vector(unsigned long n);
    Vector(unsigned long n, double v);
    Vector(const Vector& copy);

    inline unsigned long size() const noexcept { return vals_.size(); }
    double& operator[](unsigned long i) { return vals_.at(i); }
    double operator[](unsigned long i) const { return vals_.at(i); }
    double& operator()(unsigned long i) { return vals_.at(i); }
    double operator()(unsigned long i) const { return vals_.at(i); }
    double at(unsigned long i) const { return vals_.at(i); }
    void set(unsigned long i, const double& val) { vals_.at(i) = val; }

    void clear() { vals_.clear(); }

    std::string dump() const;

    std::vector<double>* get_vals() { return &vals_; }
    const std::vector<double>* get_vals() const { return &vals_; }

    inline Vector& operator+=(const Vector& rhs) {
      for (unsigned i = 0; i < vals_.size() && i < rhs.size(); ++i) {
        vals_[i] += rhs.at(i);
      }
      return *this;
    }
    inline Vector& operator-=(const Vector& rhs) {
      for (unsigned i = 0; i < vals_.size() && i < rhs.size(); ++i) {
        vals_[i] -= rhs.at(i);
      }
      return *this;
    }

   private:
    std::vector<double> vals_;
  };

  void save_vec_to_file(const std::string& file_name, const Vector& vec);
  Vector load_vec_from_file(const std::string& file_name);

  Vector operator+(const Vector& lhs, const Vector& rhs);
  Vector operator-(const Vector& lhs, const Vector& rhs);
  Vector operator*(const Vector& lhs, const double& rhs);
  Vector operator*(const double& lhs, const Vector& rhs);

  double dot(const Vector& lhs, const Vector& rhs);
  double norm(const Vector& lhs);
}  // namespace linalg
}  // namespace arta

#endif  // ARTA_MATH_VECTOR_HPP_
