#include "pde.hpp"

#include <string>

#include <sys/stat.h>
#include <unistd.h>

#include "basis.hpp"
#include "linalg.hpp"
#include "logger.hpp"
#include "mesh.hpp"
#include "plot.hpp"
#include "print.hpp"
#include "script.hpp"
#include "timer.hpp"

#include <iostream>

arta::PDE::PDE() : script_source() {}
arta::PDE::PDE(argparse::Arguments args)
    : script_source(args.options["script"]),
      mesh_source(args.options["mesh"]),
      mesh_constraints({{args.getf("mesh-area"), args.getf("mesh-angle")}}),
      timer(args.flags["time"]),
      save(!args.flags["no-save"]),
      w(args.geti("res")),
      h(args.geti("res")),
      bg(args.geth("bg")),
      cmap(args.options["cmap"]) {
  load_script();
  load_mesh();
  construct_matrices();
  construct_forcing(0.0);
}
arta::PDE::PDE(std::string script_, std::string mesh_src,
               std::array<double, 2> mesh_const, bool time)
    : script_source(script_),
      mesh_source(mesh_src),
      mesh_constraints(mesh_const),
      timer(time) {
  load_script();
  load_mesh();
}
arta::PDE::~PDE() { script::close_script(); }

void arta::PDE::construct_matrices() {
  // TODO What the actual F**k is going on here???
  if (timer) {
    time::start();
  }

  if (access((dest_dir + "G.mat").c_str(), F_OK) != -1 &&
      access((dest_dir + "M.mat").c_str(), F_OK) != -1) {
    G_ = linalg::load_mat_from_file(dest_dir + "G.mat");
    M_ = linalg::load_mat_from_file(dest_dir + "M.mat");
    linalg::save_mat_to_file(dest_dir + "Mm.mat", M_);
  } else {
    G_ = linalg::Matrix(mesh.pts.size());
    M_ = linalg::Matrix(mesh.pts.size());
    for (unsigned long ele = 0; ele < mesh.tri.size(); ++ele) {
      for (unsigned long i = 0; i < 3; ++i) {
        for (unsigned long j = 0; j < 3; ++j) {
          G_.set(mesh.tri[ele][i], mesh.tri[ele][j],
                 G_.at(mesh.tri[ele][i], mesh.tri[ele][j]) +
                     calc::integrate(G(ele, i, j), ele, &mesh));
          M_.set(mesh.tri[ele][i], mesh.tri[ele][j],
                 M_.at(mesh.tri[ele][i], mesh.tri[ele][j]) +
                     calc::integrate(A(ele, i, j), ele, &mesh) +
                     calc::integrate(B(ele, i, j), ele, &mesh) +
                     calc::integrate(C(ele, i, j), ele, &mesh));
        }
      }
    }
    // for (unsigned i = 0; i < mesh.pts.size(); ++i) {
    //   if (mesh.is_boundary(i)) {
    //     for (unsigned j = 0; j < mesh.pts.size(); ++j) {
    //       if (i == j) {
    //         M_.set(i, j, 1.0);
    //       } else {
    //         M_.set(i, j, 0.0);
    //       }
    //     }
    //   }
    // }
    if (save) {
      linalg::save_mat_to_file(dest_dir + "G.mat", G_);
      linalg::save_mat_to_file(dest_dir + "M.mat", M_);
    }
  }
  if (timer) {
    log::status("Construct Matricies: %f", time::stop());
  }
}

void arta::PDE::construct_forcing(const double& t) {
  time_ = t;
  if (timer) {
    time::start();
  }
  if (access((dest_dir + "F.vec").c_str(), F_OK) != -1) {
    F_ = linalg::load_vec_from_file(dest_dir + "F.vec");
  } else {
    F_ = linalg::Vector(mesh.pts.size(), 0.0);
    for (unsigned long ele = 0; ele < mesh.tri.size(); ++ele) {
      for (unsigned long i = 0; i < 3; ++i) {
        F_[mesh.tri[ele][i]] += calc::integrate(F(ele, i, time_), ele, &mesh);
      }
    }
    // for (unsigned i = 0; i < mesh.pts.size(); ++i) {
    //   if (mesh.is_boundary(i)) {
    //     F_[i] = script::boundary(mesh.bdry_index[i], mesh.pts[i].x,
    //                              mesh.pts[i].y, time_);
    //   }
    // }
    if (save) {
      linalg::save_vec_to_file(dest_dir + "F.vec", F_);
    }
  }
  if (timer) {
    log::status("Construct Forcing: %f", time::stop());
  }
}

void arta::PDE::construct_init() {
  time_ = 0.0;
  if (timer) {
    time::start();
  }
  if (access((dest_dir + "U0000.vec").c_str(), F_OK) != -1) {
    U_ = linalg::load_vec_from_file(dest_dir + "U0000.vec");
  } else {
    U_ = linalg::Vector(mesh.pts.size(), 0.0);
    for (unsigned long i = 0; i < mesh.pts.size(); ++i) {
      U_[i] = script::init(mesh.pts[i].x, mesh.pts[i].y);
    }
    if (save) {
      linalg::save_vec_to_file(dest_dir + "U0000.vec", U_);
    }
  }
  if (timer) {
    log::status("Construct Initial: %f", time::stop());
  }
}

void arta::PDE::apply_bc(linalg::Matrix& A) {
  for (unsigned i = 0; i < mesh.pts.size(); ++i) {
    if (mesh.is_boundary(i)) {
      for (unsigned j = 0; j < mesh.pts.size(); ++j) {
        if (i == j) {
          A.set(i, j, 1.0);
        } else {
          A.set(i, j, 0.0);
        }
      }
    }
  }
}

void arta::PDE::apply_bc(linalg::Vector& b) {
  for (unsigned i = 0; i < mesh.pts.size(); ++i) {
    if (mesh.is_boundary(i)) {
      b[i] = script::boundary(mesh.bdry_index[i], mesh.pts[i].x, mesh.pts[i].y,
                              time_);
    }
  }
}

arta::linalg::Vector arta::PDE::solve_time_indep() {
  apply_bc(M_);
  apply_bc(F_);
  if (timer) {
    time::start();
  }
  if (access((dest_dir + "U.vec").c_str(), F_OK) != -1) {
    U_ = linalg::load_vec_from_file(dest_dir + "U.vec");
  } else {
    U_ = linalg::solve(M_, F_, F_.size());
    if (save) {
      linalg::save_vec_to_file(dest_dir + "U.vec", U_);
    }
  }
  if (timer) {
    log::status("Solving Time Indep: %f", time::stop());
  }
  return U_;
}

void arta::PDE::solve_time_dep() {
  double dt = script::getd({"dt", "deltat", "delta_t"});
  unsigned N =
      static_cast<unsigned>(script::getd({"tmax", "t_max", "tm"}) / dt);
  construct_init();
  linalg::Matrix A = G_ + 0.5 * dt * M_;
  linalg::Matrix B = G_ - 0.5 * dt * M_;
  std::cout << G_.dump() << "\n\n"
            << M_.dump() << "\n\n"
            << (0.5 * dt * M_).dump() << "\n\n";
  std::cout << A.dump() << "\n\n\n=====\n";
  std::cout << B.dump() << "\n\n\n=====\n";
  apply_bc(A);
  std::vector<linalg::Vector> apxs;
  for (unsigned n = 0; n < N; ++n) {
    // plot_async(dest_dir + fmt_val(n) + ".png", U_, &mesh, w, h, cmap, bg);
    linalg::Vector F_n = F_;
    construct_forcing((n + 1) * dt);
    apxs.push_back(U_);
    if (access((dest_dir + "U" + fmt_val(n + 1) + ".vec").c_str(), F_OK) !=
        -1) {
      U_ = linalg::load_vec_from_file(dest_dir + "U" + arta::fmt_val(n + 1) +
                                      ".vec");
    } else {
      linalg::Vector C = dt / 2.0 * (F_ + F_n);
      linalg::Vector Q = B * U_ + C;
      apply_bc(Q);
      U_ = linalg::solve(A, Q);
      std::cout << Q.dump() << "\n\n" << U_.dump() << "\n\n";
      if (save) {
        linalg::save_vec_to_file(dest_dir + "U" + arta::fmt_val(n + 1) + ".vec",
                                 U_);
      }
    }
  }
  plot_async(dest_dir, apxs, &mesh, w, h, cmap, bg);
}

double arta::PDE::approx(const double& x, const double& y,
                         const unsigned& e) const {
  double val = 0.0;
  for (unsigned i = 0; i < U_.size(); ++i) {
    val += (U_[i] * basis::global(&mesh, x, y, i, e));
  }
  return val;
}

std::function<double(double, double)> arta::PDE::G(const unsigned& e,
                                                   const unsigned& i,
                                                   const unsigned& j) {
  mesh::Mesh* mesh_ptr = &mesh;
  return [mesh_ptr, e, i, j](double x, double y) {
    return basis::local(mesh_ptr, x, y, e, i) *
           basis::local(mesh_ptr, x, y, e, j);
  };
}
std::function<double(double, double)> arta::PDE::A(const unsigned& e,
                                                   const unsigned& i,
                                                   const unsigned& j) {
  mesh::Mesh* mesh_ptr = &mesh;
  return [mesh_ptr, e, i, j](double x, double y) {
    return -(calc::derivative(script::A, x, y, mesh_ptr->grain_size(e), 0.0, 1,
                              1) *
                 basis::local(mesh_ptr, x, y, e, i) +
             script::A(x, y, 1, 1) * basis::dlocal_dx(mesh_ptr, x, y, e, i)) *
               basis::dlocal_dx(mesh_ptr, x, y, e, j) -
           (calc::derivative(script::A, x, y, mesh_ptr->grain_size(e), 0.0, 1,
                             2) *
                basis::local(mesh_ptr, x, y, e, i) +
            script::A(x, y, 1, 2) * basis::dlocal_dx(mesh_ptr, x, y, e, i)) *
               basis::dlocal_dy(mesh_ptr, x, y, e, j) -
           (calc::derivative(script::A, x, y, 0.0, mesh_ptr->grain_size(e), 2,
                             1) *
                basis::local(mesh_ptr, x, y, e, i) +
            script::A(x, y, 2, 1) * basis::dlocal_dy(mesh_ptr, x, y, e, i)) *
               basis::dlocal_dx(mesh_ptr, x, y, e, j) -
           (calc::derivative(script::A, x, y, 0.0, mesh_ptr->grain_size(e), 2,
                             2) *
                basis::local(mesh_ptr, x, y, e, i) +
            script::A(x, y, 2, 2) * basis::dlocal_dy(mesh_ptr, x, y, e, i)) *
               basis::dlocal_dy(mesh_ptr, x, y, e, j);
  };
}
std::function<double(double, double)> arta::PDE::B(const unsigned& e,
                                                   const unsigned& i,
                                                   const unsigned& j) {
  mesh::Mesh* mesh_ptr = &mesh;
  return [mesh_ptr, e, i, j](double x, double y) {
    return script::B(x, y, 1) * basis::local(mesh_ptr, x, y, e, i) *
               basis::dlocal_dx(mesh_ptr, x, y, e, j) +
           script::B(x, y, 2) * basis::local(mesh_ptr, x, y, e, i) *
               basis::dlocal_dy(mesh_ptr, x, y, e, j);
  };
}
std::function<double(double, double)> arta::PDE::C(const unsigned& e,
                                                   const unsigned& i,
                                                   const unsigned& j) {
  mesh::Mesh* mesh_ptr = &mesh;
  return [mesh_ptr, e, i, j](double x, double y) {
    return script::C(x, y) * basis::local(mesh_ptr, x, y, e, i) *
           basis::local(mesh_ptr, x, y, e, j);
  };
}
std::function<double(double, double)> arta::PDE::F(const unsigned& e,
                                                   const unsigned& i,
                                                   const double& t) {
  mesh::Mesh* mesh_ptr = &mesh;
  return [mesh_ptr, e, i, t](double x, double y) {
    return basis::local(mesh_ptr, x, y, e, i) * script::forcing(x, y, t);
  };
}

void arta::PDE::load_script() {
  if (timer) {
    time::start();
  }
  script::load_script(script_source);
  if (timer) {
    log::status("Script Load: %f", time::stop());
  }
  dest_dir = "./" +
             script_source.substr(
                 script_source.rfind('/') + 1,
                 script_source.rfind('.') - script_source.rfind('/') - 1) +
             '/';
}

void arta::PDE::load_mesh() {
  if (script::has("mesh") && mesh_source == "") {
    mesh_source = script::gets("mesh");
  }
  if (script::has("mesh_area") && mesh_constraints[0] == -1) {
    mesh_constraints[0] = script::getd("mesh_area");
  }
  if (script::has("mesh_angle") && mesh_constraints[1] == -1) {
    mesh_constraints[1] = script::getd("mesh_angle");
  }
  if (mesh_constraints[0] == -1) mesh_constraints[0] = 1.0;
  if (mesh_constraints[1] == -1) mesh_constraints[1] = 10.0;
  if (mesh_source != "") {
    if (timer) {
      time::start();
    }
    std::string mesh_base_name(mesh_source);
    mesh_base_name.erase(0, mesh_base_name.find_last_of("\\/") + 1);
    mesh_base_name.erase(mesh_base_name.rfind('.'));
    std::string mesh_dest = dest_dir + mesh_base_name;
    if (access((mesh_dest + ".ele").c_str(), F_OK) != -1 &&
        access((mesh_dest + ".neigh").c_str(), F_OK) != -1 &&
        access((mesh_dest + ".node").c_str(), F_OK) != -1 &&
        access((mesh_dest + ".poly").c_str(), F_OK) != -1) {
    } else {
      mesh::construct_mesh(mesh_source, dest_dir, mesh_constraints[0],
                           mesh_constraints[1]);
    }
    mesh = mesh::Mesh(mesh_dest);
    log::info("Verts: %ld Tris: %ld", mesh.pts.size(), mesh.tri.size());
    if (timer) {
      log::status("Mesh Gen/Load: %f", time::stop());
    }
  } else {
    log::error("Must define a mesh file either by command line or by script.");
  }
}

double arta::approx(const double& x, const double& y, const unsigned& e,
                    const linalg::Vector& U, const mesh::Mesh* mesh) {
  double val = 0.0;
  for (unsigned i = 0; i < U.size(); ++i) {
    val += (U[i] * basis::global(mesh, x, y, i, e));
  }
  return val;
}
