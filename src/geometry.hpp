#ifndef FEM_GEOMETRY_HPP_
#define FEM_GEOMETRY_HPP_

namespace fem {
template <typename _T>
struct Pair {
  _T x, y;
  _T& operator[](unsigned i) { return (i == 0) ? x : y; }
  const _T& operator[](unsigned i) const { return (i == 0) ? x : y; }
};
template <typename _T>
struct Triple {
  _T x, y, z;
  _T& operator[](unsigned i) { return (i == 0) ? x : (i == 1) ? y : z; }
  const _T& operator[](unsigned i) const { return (i == 0) ? x : (i == 1) ? y : z; }
};
}  // namespace fem

#endif  // FEM_GEOMETRY_HPP_