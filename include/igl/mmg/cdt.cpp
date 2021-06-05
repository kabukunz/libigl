#include "common/Common.hpp"

#include "mmg/mmg2d/libmmg2d.h"

#include <Eigen/Core>

using namespace std;
using namespace Eigen;

bool Common::CDT2D(MatrixXd &V,  // 2D polygon vertices
                   MatrixXi &EL, // 2D polygon boundary edges indices
                   MatrixXd &VM, // 2D CDT polygon vertices OUT
                   MatrixXi &FM  // 2D CDT polygon triangles OUT
)
{
    //
    // CDT 2D
    //

    logger->Info("***** CDT 2D REMESHING *****");

    // MMG2D CDT REMESHING
    // get verts, edges and holes in
    // get verts and tris out

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

    logger->Debug("Vertices to remesh (V)", V);
    logger->Debug("Edges to remesh (EL)", EL);

    // NOTE: Eigen indices start at 0, MMGS indices start at 1

    // init mesh
    ier = MMG2D_Init_mesh(MMG5_ARG_start, MMG5_ARG_ppMesh, &mesh, MMG5_ARG_ppMet, &met, MMG5_ARG_end);
    if (!ier)
        return false;

    //
    // SET OPTIONS
    //

    // mmg2d_verbose level
    ier = MMG2D_Set_iparameter(mesh, met, MMG2D_IPARAM_verbose, mmg2d_verbose);
    if (!ier)
        return false;

    // force hard angles on border
    ier = MMG2D_Set_dparameter(mesh, met, MMG2D_DPARAM_angleDetection, mmg2d_angleDetection);
    if (!ier)
        return false;

    // do not insert steiner points
    ier = MMG2D_Set_iparameter(mesh, met, MMG2D_IPARAM_noinsert, mmg2d_noInsert);
    if (!ier)
        return false;

    // set quality
    if (FIND(Operation::REMESH, Option::mmg2d_hgrad))
    {
        ier = MMG2D_Set_dparameter(mesh, met, MMG2D_DPARAM_hgrad, mmg2d_hgrad);
        if (!ier)
            return false;
    }

    // set hsiz
    if (FIND(Operation::REMESH, Option::mmg2d_hsiz))
    {
        ier = MMG2D_Set_dparameter(mesh, met, MMG2D_DPARAM_hsiz, mmg2d_hsiz);
        if (!ier)
            return false;
    }

    //
    // SET MESH
    //

    // get verts and tris number
    np = V.rows();
    na = EL.rows();

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
    if (na && mmg2d_isEdges)
    {
        // set edges mem
        edges_in = (int *)malloc(edges_size * na * sizeof(int));

        // set edges
        for (int i = 0; i < na; i++)
        {
            int e1 = EL(i, 0) + 1;
            int e2 = EL(i, 1) + 1;

            edges_in[edges_size * i] = e1;
            edges_in[edges_size * i + 1] = e2;
        }

        // set edges
        ier = MMG2D_Set_edges(mesh, edges_in, NULL);
        if (!ier)
            return false;
    }

    //
    // REMESH
    //

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

    VM.resize(np, verts_size);
    for (int i = 0; i < np; i++)
    {
        double x = verts_out[2 * i];
        double y = verts_out[2 * i + 1];

        VM(i, 0) = x;
        VM(i, 1) = y;
    }

    // set tris
    tris_out = (int *)malloc(tris_size * nt * sizeof(int));

    // get tris
    ier = MMG2D_Get_triangles(mesh, tris_out, NULL, NULL);
    if (!ier)
        return false;

    FM.resize(nt, tris_size);
    for (int i = 0; i < nt; i++)
    {
        int v1i = tris_out[tris_size * i];
        int v2i = tris_out[tris_size * i + 1];
        int v3i = tris_out[tris_size * i + 2];

        FM(i, 0) = v1i - 1;
        FM(i, 1) = v2i - 1;
        FM(i, 2) = v3i - 1;
    }

    logger->Debug("Remeshed vertices (VM)", VM);
    logger->Debug("Remeshed faces (FM)", FM);

    // free all
    free(verts_in);
    if (na && mmg2d_isEdges)
        free(edges_in);

    free(verts_out);
    free(tris_out);

    ier = MMG2D_Free_all(MMG5_ARG_start, MMG5_ARG_ppMesh, &mesh, MMG5_ARG_ppMet, &met, MMG5_ARG_end);
    if (!ier)
        return false;

    return true;
}
