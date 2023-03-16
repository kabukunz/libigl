#include <igl/arap.h>
#include <igl/boundary_loop.h>
#include <igl/harmonic.h>
#include <igl/map_vertices_to_circle.h>
#include <igl/readOFF.h>
#include <igl/opengl/glfw/Viewer.h>

#include "tutorial_shared_path.h"

Eigen::MatrixXd V;
Eigen::MatrixXi F;
Eigen::MatrixXd V_uv;
Eigen::MatrixXd initial_guess;
Eigen::MatrixXd face_colors;

bool show_uv = false;
bool show_texture = true;


bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier)
{
  if (key == '1')
    show_uv = false;
  else if (key == '2')
    show_uv = true;

  if (key == 'q')
    V_uv = initial_guess;
  if (key == 'D' || key == 'd')
    show_texture = !show_texture;

  if (show_uv)
  {
    viewer.data().set_mesh(V_uv,F);
    viewer.core().align_camera_center(V_uv,F);
  }
  else
  {
    viewer.data().set_mesh(V,F);
    viewer.core().align_camera_center(V,F);
  }    
 
  viewer.data().show_texture = show_texture;

  viewer.data().compute_normals();

  return false;
}


int main(int argc, char *argv[])
{
  using namespace std;
  // Load a mesh in OFF format
  igl::readOFF(TUTORIAL_SHARED_PATH "/camelhead.off", V, F);

  // Compute the initial solution for ARAP (harmonic parametrization)
  Eigen::VectorXi bnd;
  igl::boundary_loop(F,bnd);
  Eigen::MatrixXd bnd_uv;
  igl::map_vertices_to_circle(V,bnd,bnd_uv);

  igl::harmonic(V,F,bnd,bnd_uv,1,initial_guess);

  // Add dynamic regularization to avoid to specify boundary conditions
  igl::ARAPData arap_data;
  arap_data.with_dynamics = true;
  Eigen::VectorXi b  = Eigen::VectorXi::Zero(0);
  Eigen::MatrixXd bc = Eigen::MatrixXd::Zero(0,0);

  // Initialize ARAP
  arap_data.max_iter = 100;
  // 2 means that we're going to *solve* in 2d
  arap_precomputation(V,F,2,b,arap_data);

  // Solve arap using the harmonic map as initial guess
  V_uv = initial_guess;

  arap_solve(bc,arap_data,V_uv);



  // Compute per-face distortion
  int nfaces = F.rows();
  face_colors.resize(nfaces, 3);
  for(int i=0; i<nfaces; i++)
  {
    Eigen::Matrix<double, 2, 3> M1;
    M1.row(0) = V.row(F(i,1)) - V.row(F(i,0));
    M1.row(1) = V.row(F(i,2)) - V.row(F(i,0));
    Eigen::Matrix2d M2;
    M2.row(0) = V_uv.row(F(i,1)) - V_uv.row(F(i,0));
    M2.row(1) = V_uv.row(F(i,2)) - V_uv.row(F(i,0));
    Eigen::Matrix2d M = Eigen::Matrix2d::Identity();
    M -= (M1*M1.transpose()).inverse() * M2*M2.transpose();
    Eigen::Vector2cd evals = M.eigenvalues();
    
    double lmax = std::max(std::real(evals[0]), std::real(evals[1]));
    double lmin = std::min(std::real(evals[0]), std::real(evals[1]));
    double magic = 0.5;
    face_colors(i,0) = 1.0 - magic*std::max(0.0, -lmin);
    face_colors(i,1) = 1.0 - magic*std::max(lmax, -lmin);
    face_colors(i,2) = 1.0 - magic*std::max(0.0, lmax);
  }



  // Scale UV to make the texture more clear
  V_uv *= 20;
  
  // Plot the mesh
  igl::opengl::glfw::Viewer viewer;
  viewer.data().set_mesh(V, F);
  viewer.data().set_uv(V_uv);
  viewer.callback_key_down = &key_down;

  // Disable wireframe
  viewer.data().show_lines = false;

  // face colors
  viewer.data().set_colors(face_colors);
  viewer.data().set_face_based(true);

//   // Draw checkerboard texture
//   viewer.data().show_texture = true;

  // Launch the viewer
  viewer.launch();
}
