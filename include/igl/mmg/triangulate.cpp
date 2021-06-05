// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "mmg/mmg2d/libmmg2d.h"

template <
 typename DerivedV,
 typename DerivedE,
 typename DerivedH, 
 typename DerivedV2,
 typename DerivedF2>
IGL_INLINE bool igl::mmg::triangulate(
  const Eigen::MatrixBase<DerivedV> & V,
  const Eigen::MatrixBase<DerivedE> & E,
  const Eigen::MatrixBase<DerivedH> & H,
  Eigen::PlainObjectBase<DerivedV2> & V2,
  Eigen::PlainObjectBase<DerivedF2> & F2
  )
{

    // V, E are required for Scaf
    assert(V.rows() > 0 && "Vertices matrix is empty");
    assert(E.rows() > 0 && "Edges matrix is empty");

    // CHECK: Scaf does not support holes 
    // and does virtual hole fill if any is found,
    // so we do not use holes here for now

    // vars
    MMG5_pMesh mesh = NULL;
    MMG5_pSol met = NULL;
    int np = {};
    int nt = {};
    int nquad = {};
    int na = {};
    int verts_size = 2;
    int edges_size = 2;
    int tris_size = 3;
    double *verts_in = NULL;
    int *edges_in = NULL;
    double *verts_out = NULL;
    int *tris_out = NULL;
    int ier = {};

    // NOTE: Eigen indices start at 0, MMGS indices start at 1

    // init mesh
    ier = MMG2D_Init_mesh(MMG5_ARG_start, MMG5_ARG_ppMesh, &mesh, MMG5_ARG_ppMet, &met, MMG5_ARG_end);
    if (!ier)
        return false;

    int mmg2d_verbose = -1;
    double mmg2d_angleDetection = 0.0;
    int mmg2d_noInsert = 1;
    double mmg2d_hgrad = 30.0;

    // mmg2d_verbose level (-1 is silent)
    ier = MMG2D_Set_iparameter(mesh, met, MMG2D_IPARAM_verbose, mmg2d_verbose);
    if (!ier)
        return false;

    // force hard angles on border (do not modify scaffold squared edges)
    ier = MMG2D_Set_dparameter(mesh, met, MMG2D_DPARAM_angleDetection, mmg2d_angleDetection);
    if (!ier)
        return false;

    // do not insert steiner points (do not modify original scaffold vertices)
    ier = MMG2D_Set_iparameter(mesh, met, MMG2D_IPARAM_noinsert, mmg2d_noInsert);
    if (!ier)
        return false;

    // remeshing quality (as Delaunay as possible scaffold triangular distribution)
    ier = MMG2D_Set_dparameter(mesh, met, MMG2D_DPARAM_hgrad, mmg2d_hgrad);
    if (!ier)
        return false;

    // get verts and tris number
    np = V.rows();
    na = E.rows();

    // create mesh: vertices and edges
    ier = MMG2D_Set_meshSize(mesh, np, nt, nquad, na);
    if (!ier)
        return false;

    // set vertices mem
    verts_in = (double *)malloc(verts_size * np * sizeof(double));

    // set vertices
    for (int i = 0; i < np; i++)
    {
        double x = V(i, 0);
        double y = V(i, 1);

        verts_in[verts_size * i] = x;
        verts_in[verts_size * i + 1] = y;
    }
    // set vertices
    MMG2D_Set_vertices(mesh, verts_in, NULL);

    // insert edges as single border
    if (na)
    {
        // set edges mem
        edges_in = (int *)malloc(edges_size * na * sizeof(int));

        // set edges
        for (int i = 0; i < na; i++)
        {
            int e1 = E(i, 0) + 1;
            int e2 = E(i, 1) + 1;

            edges_in[edges_size * i] = e1;
            edges_in[edges_size * i + 1] = e2;
        }

        // set edges
        ier = MMG2D_Set_edges(mesh, edges_in, NULL);
        if (!ier)
            return false;
    }

    // generate a regular fine mesh of the square in meshing mode
    ier = MMG2D_mmg2dmesh(mesh, met);
    if (ier != MMG5_SUCCESS)
        return false;

    // get mesh
    MMG2D_Get_meshSize(mesh, &np, &nt, &nquad, &na);

    // set vertices
    verts_out = (double *)malloc(verts_size * np * sizeof(double));

    // get vertices
    MMG2D_Get_vertices(mesh, verts_out, NULL, NULL, NULL);

    V2.resize(np, verts_size);
    for (int i = 0; i < np; i++)
    {
        double x = verts_out[2 * i];
        double y = verts_out[2 * i + 1];

        V2(i, 0) = x;
        V2(i, 1) = y;
    }

    // set tris
    tris_out = (int *)malloc(tris_size * nt * sizeof(int));

    // get tris
    ier = MMG2D_Get_triangles(mesh, tris_out, NULL, NULL);
    if (!ier)
        return false;

    F2.resize(nt, tris_size);
    for (int i = 0; i < nt; i++)
    {
        int v1i = tris_out[tris_size * i];
        int v2i = tris_out[tris_size * i + 1];
        int v3i = tris_out[tris_size * i + 2];

        F2(i, 0) = v1i - 1;
        F2(i, 1) = v2i - 1;
        F2(i, 2) = v3i - 1;
    }

    // free all
    free(verts_in);
    if (na)
        free(edges_in);

    free(verts_out);
    free(tris_out);

    ier = MMG2D_Free_all(MMG5_ARG_start, MMG5_ARG_ppMesh, &mesh, MMG5_ARG_ppMet, &met, MMG5_ARG_end);
    if (!ier)
        return false;

    return true;

}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
template bool igl::mmg::triangulate<
    Eigen::Matrix<double, -1, -1, 1, -1, -1>, 
    Eigen::Matrix<int, -1, -1, 0, -1, -1>, 
    Eigen::Matrix<double, -1, -1, 1, -1, -1>, 
    Eigen::Matrix<double, -1, -1, 1, -1, -1>, 
    Eigen::Matrix<int, -1, -1, 0, -1, -1> >(
    Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> > const&, 
    Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> >&);
#endif
