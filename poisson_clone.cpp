/*
Reilly Bova '20
COS 526: Assignment 1

poisson_clone.cpp
Clones src into dest based on mask using seamless Poisson cloning as described
in Perez et al. in 2003
*/

#include <cmath>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_splinalg.h>

#include "./lib/imageio++.h"

/*******************************************************************************
Helper functions
*******************************************************************************/

/* Returns true if pixel is whitish */
inline bool isWhite(Color &pixel)
{
  if (pixel.r > 240 && pixel.g > 240 && pixel.b > 240) {
    return true;
  }

  return false;
}

/* Return a [N, E, S, W] array of int arrays containg a pair of values:
* The first index denotes the "status" of the pixel q in the corresponding
* cardinal direction to p, and the second index will be q's index (potentially
* out of bounds). The status type are as follows:
*    1 -> in Omega
*    0 -> in boundary of Omega
*   -1 -> not in image
*
* NB: p must be valid, as there is no boundary checking
*/
inline std::vector< std::vector<int> > getNeighbors(int p, int w, int h, ::std::vector<int> &toOmega)
{
  std::vector< std::vector<int> > results (4, std::vector<int>(3, 0));
  int q;

  // North
  q = p - w;
  results[0][1] = q;
  if (q >= 0) {
    if (toOmega[q] != -1) {
      // q is in Omega
      results[0][0] = 1;
    } else {
      // q is in boundary of Omega
      results[0][0] = 0;
    }
  } else {
    // q is outside image
    results[0][0] = -1;
  }

  // East
  q = p + 1;
  results[1][1] = q;
  if ((q % w) != 0) {
    if (toOmega[q] != -1) {
      // q is in Omega
      results[1][0] = 1;
    } else {
      // q is in boundary of Omega
      results[1][0] = 0;
    }
  } else {
    // q is outside image
    results[1][0] = -1;
  }

  // South
  q = p + w;
  results[2][1] = q;
  if (((int) q / w) < h) {
    if (toOmega[q] != -1) {
      // q is in Omega
      results[2][0] = 1;
    } else {
      // q is in boundary of Omega
      results[2][0] = 0;
    }
  } else {
    // q is outside image
    results[2][0] = -1;
  }

  // West
  q = p - 1;
  results[3][1] = q;
  if ((p % w) != 0) {
    if (toOmega[q] != -1) {
      // q is in Omega
      results[3][0] = 1;
    } else {
      // q is in boundary of Omega
      results[3][0] = 0;
    }
  } else {
    // q is outside image
    results[3][0] = -1;
  }

  return results;
}

/* Returns the guidance v between p and q
* Rmk: computed as g(p) - g(q)
* NB: Does not check boundaries
*/
inline double guidance(Im &src, Im &dest, int p, int q, int xOff, int yOff, int channel, int mode)
{
  // Take into account offsets; set gradient to 0 if either pixel goes outside!
  int W = src.w();
  int H = src.h();
  int destW = dest.w();

  // Coords in Dest
  int px = p % destW;
  int py = p / destW;
  int qx = q % destW;
  int qy = q / destW;

  // Coords in src
  int pu = px - xOff;
  int pv = py - yOff;
  int qu = qx - xOff;
  int qv = qy - yOff;

  // Check boundaries
  if (pu < 0 || pv < 0 || qu < 0 || qv < 0 || pu >= W || qu >= W || pv >= H || qv >= H) {
    return 0;
  }

  if (mode == 1) {
    // mixed mode
    double gradg = (((double) src(pu, pv)[channel]) - ((double) src(qu, qv)[channel])) / 255.0;
    double gradf = (((double) dest[p][channel]) - ((double) dest[q][channel])) / 255.0;
    if (abs(gradf) > abs(gradg)) {
      return gradf;
    } else {
      return gradg;
    }
  } else {
    // default (mode == 0 presumably)
    return (((double) src(pu, pv)[channel]) - ((double) src(qu, qv)[channel])) / 255.0;
  }
}

/* Set channel of pixel p in dest to value v in range [0-1] */
inline void setPixel(Im &dest, int p, double v, int channel) {
  // Scale and Clamp
  v *= 255.0;
  if (v > 255) {
    v = 255;
  } else if (v < 0){
    v = 0;
  }

  dest[p][channel] = (unsigned char) v;
  return;
}

/* Solve a sparse linear system of equations of form Ax = b.
* Code sourced from docs: https://www.gnu.org/software/gsl/doc/html/splinalg.html
*/
inline int solve(gsl_spmatrix *A, gsl_vector *x, gsl_vector *b, int OMEGA_SIZE)
{
  const double tol = 1.0e-6;  /* solution relative tolerance */
  const size_t max_iter = 1000; /* maximum iterations */
  const gsl_splinalg_itersolve_type *T = gsl_splinalg_itersolve_gmres;
  gsl_splinalg_itersolve *work = gsl_splinalg_itersolve_alloc(T, OMEGA_SIZE, 0);
  size_t iter = 0;
  double residual;
  int status;

  /* initial guess x = 0 */
  gsl_vector_set_zero(x);

  /* solve the system Ax = b */
  do {
    status = gsl_splinalg_itersolve_iterate(A, b, tol, x, work);

    /* print out residual norm ||A*x - b|| */
    if (iter % 100 == 0) {
      residual = gsl_splinalg_itersolve_normr(work);
      fprintf(stderr, "iter %zu residual = %.12e\n", iter, residual);
    }

    if (status == GSL_SUCCESS)
    fprintf(stderr, "Converged\n");
  } while (status == GSL_CONTINUE && ++iter < max_iter);

  gsl_splinalg_itersolve_free(work);
  return status;
}

/* Converts an image to monochrome using luminosity (does not overwrite im) */
inline Im imToMonochrome(Im im)
{
  for (int y = im.h() - 1; y >= 0; y--) {
    for (int x = im.w() - 1; x >= 0; x--) {
      // Compute monochrome value and clamp
      Color &pixel = im(x, y);
      double mono = 0.21*pixel[0] + 0.72*pixel[1] + 0.07*pixel[2];
      if (mono > 255) {
        mono = 255;
      } else if (mono < 0){
        mono = 0;
      }
      // Cast and set
      unsigned char m = (unsigned char) mono;
      for (int j = 0; j < 3; j++)
      pixel[j] = m;
    }
  }

  return im;
}

/*******************************************************************************
Poisson Seamless Cloning
*******************************************************************************/

// Implements poisson seamless cloning
inline int poisson_clone(Im &src, Im &mask, Im dest, int xOff, int yOff, const char* outfilename, int mode)
{
  // Number of pixels in dest and mask
  int W = dest.w();
  int H = dest.h();
  int N = W*H;

  printf("Poisson cloning...\n");

  /* Map pixel indices to Omega membership, given by ID <id>. An ID of -1 implies
  *  the pixel lies outside a mask (it may still be a boundary pixel though) */
  ::std::vector<int> toOmega (N, -1);
  int id = 0;
  for (int i = N - 1; i >= 0; i--) {
    if (isWhite(mask[i])) {
      toOmega[i] = id;
      id++;
    }
  }

  /* Now reverse the mapping now that we know how many ids we have */
  int OMEGA_SIZE = id;
  ::std::vector<int> toMask (OMEGA_SIZE);
  for (int i = N - 1; i >= 0; i--) {
    if (toOmega[i] >= 0) {
      toMask[toOmega[i]] = i;
    }
  }

  /* Initialize system of equations */
  printf("Setting up system of equations...\n");
  // RHS
  gsl_vector *r = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known reds" */
  gsl_vector *g = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known greens" */
  gsl_vector *b = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known blues" */

  /* Sparse matrix of coefficients (LHS) */
  gsl_spmatrix *A = gsl_spmatrix_alloc(OMEGA_SIZE, OMEGA_SIZE);
  gsl_spmatrix *C;                               /* compressed format */
  gsl_vector *x = gsl_vector_alloc(OMEGA_SIZE);  /* vector for solutions (LHS) */

  /* Iterate through the pixels in Omega... */
  for (int id = OMEGA_SIZE - 1; id >= 0; id--) {
    int p = toMask[id]; // Pixel index in dest and mask
    int Np = 0;         // Number of cardinal neighbors in image
    double r_val = 0.0; // RHS of equation
    double g_val = 0.0; // RHS of equation
    double b_val = 0.0; // RHS of equation

    std::vector< std::vector<int> > neighbors = getNeighbors(p, W, H, toOmega);

    // For each neighbor q....
    for (int j = 0; j < 4; j++) {
      int status = neighbors[j][0];
      int q = neighbors[j][1];

      // Ignore pixels outside the image
      if (status == -1) {
        continue;
      }

      Np++; // Count the neighbor
      r_val += guidance(src, dest, p, q, xOff, yOff, 0, mode); // Guidance constraint
      g_val += guidance(src, dest, p, q, xOff, yOff, 1, mode); // Guidance constraint
      b_val += guidance(src, dest, p, q, xOff, yOff, 2, mode); // Guidance constraint

      // For q in Omega
      if (status == 1) {
        // -fq component
        int q_id = toOmega[q];
        gsl_spmatrix_set(A, id, q_id, -1.0);
      }
      // For q in boundary of Omega
      if (status == 0) {
        // f* boundary constraint
        r_val += (double) dest[q][0]/255.0;
        g_val += (double) dest[q][1]/255.0;
        b_val += (double) dest[q][2]/255.0;
      }
    }

    // Np*fp component
    gsl_spmatrix_set(A, id, id, (double) Np);
    // Record constraints
    gsl_vector_set(r, id, r_val);
    gsl_vector_set(g, id, g_val);
    gsl_vector_set(b, id, b_val);
  }

  /* convert to compressed column format */
  C = gsl_spmatrix_ccs(A);

  /* Sparsely solve the systems of equations for each channel */
  for (int c = 0; c < 3; c++) {
    printf("Solving for channel %d\n", c);
    if (c == 0) solve(C, x, r, OMEGA_SIZE);
    else if (c == 1) solve(C, x, g, OMEGA_SIZE);
    else if (c == 2) solve(C, x, b, OMEGA_SIZE);

    /* Copy into dest */
    for (int j = OMEGA_SIZE - 1; j >= 0; j--) {
      setPixel(dest, toMask[j], gsl_vector_get(x, j), c);
    }
  }

  /* Free mem */
  gsl_spmatrix_free(A);
  gsl_spmatrix_free(C);
  gsl_vector_free(x);
  gsl_vector_free(r);
  gsl_vector_free(g);
  gsl_vector_free(b);

  /* Write image back out */
  if (!dest.write(outfilename)){
    fprintf(stderr, "Error: poisson cloning write failed\n");
    return 1;
  }

  return 0;
}

/*******************************************************************************
Direct Cloning
*******************************************************************************/

// Implements direct [seamed] cloning
inline int direct_clone(Im &src, Im &mask, Im dest, int xOff, int yOff, const char* outfilename)
{
  // Number of pixels in dest and mask
  int W = dest.w();
  int H = dest.h();

  int srcW = src.w();
  int srcH = src.h();

  printf("Direct cloning...\n");

  /* Clone masked region from src to dest */
  for (int y = H - 1; y >= 0; y--) {
    for (int x = W - 1; x >= 0; x--) {
      // Copy src onto dest if mask is white
      if (isWhite(mask(x, y))) {
        int u = x - xOff;
        int v = y - yOff;
        if (0 <= u && 0 <= v && u < srcW && v < srcH) {
          Color &src_pixel = src(u, v);
          Color &dest_pixel = dest(x, y);

          for (int j = 0; j < 3; j++)
          dest_pixel[j] = (unsigned char) src_pixel[j];
        }
      }
    }
  }

  /* Write image back out */
  if (!dest.write(outfilename)) {
    fprintf(stderr, "Error: direct cloning write failed\n");
    return 1;
  }

  return 0;
}

/*******************************************************************************
Main
*******************************************************************************/

/* Main function sample usage:
* $ ./poisson_clone ./test_images/perez-fig4a-src.png ./test_images/perez-fig4a-mask.png ./test_images/perez-fig4a-dst.png 0 0 direct.png -direct
* $ ./poisson_clone ./test_images/perez-fig4a-src-orig.png ./test_images/perez-fig4a-mask.png ./test_images/perez-fig4a-dst.png out.png-11 52
*
* $ ./poisson_clone ./test_images/perez-fig3a-src-orig.png ./test_images/perez-fig3a-mask.png ./test_images/perez-fig3a-dst.png out.png -33 -33
*
* $ ./poisson_clone ./test_images/perez-fig3b-src1-orig.png ./test_images/perez-fig3b-mask1.png ./test_images/perez-fig3b-dst.png out.png 33 24
* $ ./poisson_clone ./test_images/perez-fig3b-src2-orig.png ./test_images/perez-fig3b-mask2.png ./out.png out.png 20 110
* $ ./poisson_clone ./test_images/perez-fig3b-src2-orig.png ./test_images/perez-fig3b-mask3.png ./out.png out.png -67 98
* $ nice -20 ./poisson_clone ./test_images/perez-fig5-src.png ./test_images/perez-fig5-mask.png ./test_images/perez-fig5-dst.png ./results/fig5_mono.png -40 52 -mono
* $ nice -20 ./poisson_clone ./test_images/perez-fig6-src.png ./test_images/perez-fig6-mask.png ./test_images/perez-fig6-dst.png ./results/fig6_mixed.png 25 20 -mx
*/
int main(int argc, char *argv[])
{
  if (argc < 7) {
    fprintf(stderr, "Usage: %s src.png mask.png dest.png out.png xOffset yOffset [(-d || -direct) || (-mono || -monochrome)]\n", argv[0]);
    exit(1);
  }
  const char *srcfilename = argv[1];
  const char *maskfilename = argv[2];
  const char *destfilename = argv[3];
  const char *outfilename = argv[4];
  const int xOff = atoi(argv[5]);
  const int yOff = atoi(argv[6]);

  /* Read the src image */
  Im src;
  if (!src.read(srcfilename))
  exit(1);

  /* Read the dest image */
  Im dest;
  if (!dest.read(destfilename))
  exit(1);

  /* Read the dest image */
  Im mask;
  if (!mask.read(maskfilename))
  exit(1);

  printf("Read images of size %d x %d\n", dest.w(), dest.h());

  // Enforce equality between dims of dest and mask
  if (dest.h() != mask.h() && dest.w() != mask.w()) {
    fprintf(stderr, "Usage: dest and mask images must have identical dimensions \n");
    exit(1);
  }

  // Flags
  std::string d_short = "-d";
  std::string d_long = "-direct";
  std::string mono_short = "-mono";
  std::string mono_long = "-monochrome";
  std::string mx_short = "-mx";
  std::string mx_long = "-mixed";

  // Use flag to determine cloning method
  int error = 0;
  if (argc == 8 && (d_short.compare(argv[7]) == 0 || d_long.compare(argv[7]) == 0)) {
    // Apply direct cloning
    error = direct_clone(src, mask, dest, xOff, yOff, outfilename);
    if (error) exit(1);
  } else if (argc == 8 && (mono_short.compare(argv[7]) == 0 || mono_long.compare(argv[7]) == 0)) {
    // Convert src to monochrome and then apply poisson cloning
    src = imToMonochrome(src);
    int error = poisson_clone(src, mask, dest, xOff, yOff, outfilename, 0);
    if (error) exit(1);
  } else if (argc == 8 && (mx_short.compare(argv[7]) == 0 || mx_long.compare(argv[7]) == 0)) {
    // Apply poisson cloning in mixed mode
    int error = poisson_clone(src, mask, dest, xOff, yOff, outfilename, 1);
    if (error) exit(1);
  } else {
    // Apply Poisson seamless cloning
    int error = poisson_clone(src, mask, dest, xOff, yOff, outfilename, 0);
    if (error) exit(1);
  }

  exit(0);
}
