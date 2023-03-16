// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2021 AT3D
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef IGL_MMG_TRIANGULATE_H
#define IGL_MMG_TRIANGULATE_H
#include "../igl_inline.h"
#include <Eigen/Core>

#include <optional>

namespace igl
{
namespace mmg
{
namespace mmg2d
{

// MMG data structure
struct MMGOptions
{        
    std::optional<int> mmg2d_verbose = -1;
    std::optional<int> mmg2d_noInsert = 1;
    std::optional<double> mmg2d_hausd = {};
    std::optional<double> igl_mmg2d_hmin = {};
    std::optional<double> igl_mmg2d_hmax = {};
    std::optional<double> mmg2d_hgrad = 1.0;
    std::optional<double> mmg2d_hsiz = {};
    std::optional<double> mmg2d_angleDetection = {}; 
    std::optional<int> mmg2d_nreg = {};
    std::optional<int> mmg2d_xreg = {};
    std::optional<int> mmg2d_iter = 1;
};

// Triangulate the interior of a polygon using the mmg library.
//
// Inputs:
//   V #V by 2 list of 2D vertex positions
//   E #E by 2 list of vertex ids forming unoriented edges of the boundary of the polygon
//   H #H by 2 coordinates of points contained inside holes of the polygon
// Outputs:
//   V2   #V2 by 2  coordinates of the vertives of the generated triangulation
//   F2   #F2 by 3  list of indices forming the faces of the generated triangulation
//   bool #result   triangulation result, can be false because of constraints or other errors

template <
typename DerivedV,
typename DerivedE,
typename DerivedH,
typename DerivedV2,
typename DerivedF2>
IGL_INLINE bool triangulate(
    const Eigen::MatrixBase<DerivedV> & V,
    const Eigen::MatrixBase<DerivedE> & E,
    const Eigen::MatrixBase<DerivedH> & H,
    Eigen::PlainObjectBase<DerivedV2> & V2,
    Eigen::PlainObjectBase<DerivedF2> & F2,
    MMGOptions &mmgOptions);
}
}
}

#ifndef IGL_STATIC_LIBRARY
#  include "triangulate.cpp"
#endif

#endif
