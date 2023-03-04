// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "triangulate.h"

#ifdef LIBIGL_WITH_MMG
#include "mmg/mmg2d/libmmg2d.h"
#endif

template <
    typename DerivedV,
    typename DerivedE,
    typename DerivedH,
    typename DerivedV2,
    typename DerivedF2>
IGL_INLINE bool igl::mmg::mmg2d::triangulate(
    const Eigen::MatrixBase<DerivedV> &V,
    const Eigen::MatrixBase<DerivedE> &E,
    const Eigen::MatrixBase<DerivedH> &H,
    Eigen::PlainObjectBase<DerivedV2> &V2,
    Eigen::PlainObjectBase<DerivedF2> &F2,
    MMGOptions &mmgOptions)
{

    // V, E are required for Scaf
    assert(V.rows() > 0 && "Vertices matrix is empty");
    assert(E.rows() > 0 && "Edges matrix is empty");

#ifdef LIBIGL_WITH_MMG

    // vars
    MMG5_pMesh mmgMesh = NULL;
    MMG5_pSol mmgSol = NULL;
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

    // init mmgMesh
    ier = MMG2D_Init_mesh(MMG5_ARG_start, MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol, MMG5_ARG_end);
    if (!ier)
        return false;

    // check MMG options

    // mmg2d_verbose level
    if (mmgOptions.mmg2d_verbose)
    {
        ier = MMG2D_Set_iparameter(mmgMesh, mmgSol, MMG2D_IPARAM_verbose, mmgOptions.mmg2d_verbose.value());
        if (!ier)
            return false;
    }

    // do not insert steiner points
    if (mmgOptions.mmg2d_noInsert)
    {
        ier = MMG2D_Set_iparameter(mmgMesh, mmgSol, MMG2D_IPARAM_noinsert, mmgOptions.mmg2d_noInsert.value());
        if (!ier)
            return false;
    }

    // angle detection on borders
    if (mmgOptions.mmg2d_angleDetection)
    {
        ier = MMG2D_Set_dparameter(mmgMesh, mmgSol, MMG2D_DPARAM_angleDetection, mmgOptions.mmg2d_angleDetection.value());
        if (!ier)
            return false;
    }

    // remeshing quality
    if (mmgOptions.mmg2d_hgrad)
    {
        ier = MMG2D_Set_dparameter(mmgMesh, mmgSol, MMG2D_DPARAM_hgrad, mmgOptions.mmg2d_hgrad.value());
        if (!ier)
            return false;
    }

    // remeshing accuracy
    if (mmgOptions.mmg2d_hausd)
    {
        ier = MMG2D_Set_dparameter(mmgMesh, mmgSol, MMG2D_DPARAM_hausd, mmgOptions.mmg2d_hausd.value());
        if (!ier)
            return false;
    }

    // remeshing size
    if (mmgOptions.mmg2d_hsiz)
    {
        ier = MMG2D_Set_dparameter(mmgMesh, mmgSol, MMG2D_DPARAM_hsiz, mmgOptions.mmg2d_hsiz.value());
        if (!ier)
            return false;
    }

    // normals smoothing
    if (mmgOptions.mmg2d_nreg)
    {
        ier = MMG2D_Set_iparameter(mmgMesh, mmgSol, MMG2D_IPARAM_nreg, mmgOptions.mmg2d_nreg.value());
        if (!ier)
            return false;
    }

    // vertices smoothing
    if (mmgOptions.mmg2d_xreg)
    {
        ier = MMG2D_Set_iparameter(mmgMesh, mmgSol, MMG2D_IPARAM_xreg, mmgOptions.mmg2d_xreg.value());
        if (!ier)
            return false;
    }

    // get verts and tris number
    np = V.rows();
    na = E.rows();

    // create mmgMesh: vertices and edges
    ier = MMG2D_Set_meshSize(mmgMesh, np, nt, nquad, na);
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
    MMG2D_Set_vertices(mmgMesh, verts_in, NULL);

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
        ier = MMG2D_Set_edges(mmgMesh, edges_in, NULL);
        if (!ier)
            return false;
    }

    // remesh
    ier = MMG2D_mmg2dmesh(mmgMesh, mmgSol);
    if (ier != MMG5_SUCCESS)
        return false;

    // get mmgMesh
    MMG2D_Get_meshSize(mmgMesh, &np, &nt, &nquad, &na);

    // set vertices
    verts_out = (double *)malloc(verts_size * np * sizeof(double));

    // get vertices
    MMG2D_Get_vertices(mmgMesh, verts_out, NULL, NULL, NULL);

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
    ier = MMG2D_Get_triangles(mmgMesh, tris_out, NULL, NULL);
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

    ier = MMG2D_Free_all(MMG5_ARG_start, MMG5_ARG_ppMesh, &mmgMesh, MMG5_ARG_ppMet, &mmgSol, MMG5_ARG_end);
    if (!ier)
        return false;

#endif

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
    Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> > const&, 
    Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, 
    Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> > const&, 
    Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 1, -1, -1> >&, 
    Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> >&);
#endif
