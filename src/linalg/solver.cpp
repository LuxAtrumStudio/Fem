#include "solver.hpp"

#include <cmath>

#include "../logger.hpp"
#include "matrix.hpp"
#include "vector.hpp"

#include <iostream>

bool arta::linalg::diag_dominant(const Matrix& A) {
  for (unsigned i = 0; i < A.size(); ++i) {
    double sum = 0.0;
    for (unsigned j = 0; j < A.size(); ++j) {
      if (i == j) continue;
      sum += std::fabs(A.at(i, j));
    }
    if (std::fabs(A.at(i, i)) < sum) return false;
  }
  return true;
}

arta::linalg::Vector arta::linalg::gauss_seidel(const Matrix& A,
                                                const Vector& b,
                                                const unsigned& n) {
  Vector x(b.size());
  for (unsigned k = 0; k < n; ++k) {
    for (unsigned i = 0; i < b.size(); ++i) {
      double sigma = 0.0;
      for (unsigned j = 0; j < b.size(); ++j) {
        if (i == j) continue;
        sigma += A.at(i, j) * x.at(j);
      }
      x.set(i, (b.at(i) - sigma) / A.at(i, i));
    }
    if (norm(A * x - b) <= 1e-20) break;
  }
  return x;
}

arta::linalg::Vector arta::linalg::conjugate_gradient(const Matrix& A,
                                                      const Vector& b,
                                                      const unsigned& n) {
  Vector x(b.size());
  Vector r = b - A * x;
  Vector p = r;
  double rho_prev = dot(r, r);
  for (unsigned i = 0; i < n && rho_prev > 1e-20; ++i) {
    Vector Ap = A * p;
    double alpha = rho_prev / dot(p, Ap);
    x += p * alpha;
    r -= Ap * alpha;
    double rho_new = dot(r, r);
    if (rho_new < 1e-20) {
      break;
    }
    p = r + p * (rho_new / rho_prev);
    rho_prev = rho_new;
  }
  return x;
}

arta::linalg::Vector arta::linalg::solve(const Matrix& A, const Vector& b,
                                         const unsigned& n) {
  // TODO Conjugate Gradient only works for symmetric and positive definite!
  // Gauss-Seidel is working because it is diagonal dominant.
  // I will need to use GMRES to solve it any faster. This means leaning more
  // about the Krylov subspace, and the Arnoldi iterations.
  // return conjugate_gradient(A, b, n);
  return gauss_seidel(A, b, n);
}
