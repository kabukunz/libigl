// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
// Copyright (C) 2017 Alec Jacobson
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef IGL_MMG_TRIANGULATE_H
#define IGL_MMG_TRIANGULATE_H
#include "../igl_inline.h"
#include <string>
#include <Eigen/Core>

namespace igl
{
  namespace mmg
  {
    // Triangulate the interior of a polygon using the mmg2d library.
    //
    // Inputs:
    //   V #V by 2 list of 2D vertex positions
    //   E #E by 2 list of vertex ids forming unoriented edges of the boundary of the polygon
    //   H #H by 2 coordinates of points contained inside holes of the polygon
    //   flags  string of options pass to triangle (see triangle documentation)
    // Outputs:
    //   bool #triangulation result, can be false because of constraints or other errors
    //   V2  #V2 by 2  coordinates of the vertives of the generated triangulation
    //   F2  #F2 by 3  list of indices forming the faces of the generated triangulation
    
    template <
      typename DerivedV,
      typename DerivedE,
      typename DerivedV2,
      typename DerivedF2>
    IGL_INLINE bool triangulate(
      const Eigen::MatrixBase<DerivedV> & V,
      const Eigen::MatrixBase<DerivedE> & E,
      Eigen::PlainObjectBase<DerivedV2> & V2,
      Eigen::PlainObjectBase<DerivedF2> & F2);
        
  }
}

#ifndef IGL_STATIC_LIBRARY
#  include "triangulate.cpp"
#endif

#endif
