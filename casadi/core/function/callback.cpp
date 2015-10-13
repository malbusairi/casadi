/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "callback_internal.hpp"

using namespace std;

namespace casadi {

  Callback::Callback() {
  }

  Callback::Callback(const Callback& obj) {
    casadi_error("Callback objects cannot be copied");
  }

  void Callback::construct(const std::string& name, const Dict& opts) {
    assignNode(new CallbackInternal(name, this));
    setOption(opts);
    Function::init(false);
  }

  Callback::~Callback() {
    // Make sure that this object isn't used after its deletion
    if (!isNull()) {
      (*this)->self_ = 0;
    }
  }

  Function Callback::fun(const std::string& name, Callback* n, const Dict& opts) {
    n->construct(name, opts);
    Function ret = *n;
    n->transfer_ownership();
    return ret;
  }

  std::vector<DMatrix> Callback::eval(const std::vector<DMatrix>& arg) {
    casadi_error("Callback::eval has not been implemented");
    return std::vector<DMatrix>();
  }

  void Callback::eval(const double** arg, double** res, int* iw, double* w) {
    // Allocate input matrices
    int num_in = n_in();
    std::vector<DMatrix> argv(num_in);
    for (int i=0; i<num_in; ++i) {
      argv[i] = DMatrix::zeros(input(i).sparsity());
      if (arg[i] != 0) {
        argv[i].setNZ(arg[i]);
      } else {
        argv[i].set(0.);
      }
    }

    // Evaluate
    std::vector<DMatrix> resv = eval(argv);

    // Get the outputs
    int num_out = n_out();
    for (int i=0; i<num_out; ++i) {
      if (res[i]!=0) resv[i].getNZ(res[i]);
    }
  }

  const CallbackInternal* Callback::operator->() const {
    return static_cast<const CallbackInternal*>(Function::operator->());
  }

  CallbackInternal* Callback::operator->() {
    return static_cast<CallbackInternal*>(Function::operator->());
  }

  bool Callback::has_jacobian() const {
    return (*this)->FunctionInternal::hasFullJacobian();
  }

  Function Callback::get_jacobian(const std::string& name, const Dict& opts) {
    return (*this)->FunctionInternal::getFullJacobian(name, opts);
  }

  Function Callback::get_forward(const std::string& name, int nfwd, Dict& opts) {
    return (*this)->FunctionInternal::getDerForward(name, nfwd, opts);
  }

  int Callback::get_n_forward() const {
    return (*this)->FunctionInternal::numDerForward();
  }

  Function Callback::get_reverse(const std::string& name, int nadj, Dict& opts) {
    return (*this)->FunctionInternal::getDerReverse(name, nadj, opts);
  }

  int Callback::get_n_reverse() const {
    return (*this)->FunctionInternal::numDerReverse();
  }

  void Callback::transfer_ownership() {
    casadi_assert_message(!isNull(), "Null pointer.");
    casadi_assert_message(!(*this)->own_, "Ownership has already been transferred.");
    casadi_assert_message(getCount()>1, "There are no owning references");
    // Decrease the reference counter to offset the effect of the owning reference
    count_down();
    // Mark internal class as owning
    (*this)->own_ = true;
  }

} // namespace casadi
