/*
Reilly Bova '20
COS 526: Assignment 1

poisson_clone.cpp
Clones src into dest based on mask using seamless Poisson cloning as described
in Perez et al. in 2004
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_splinalg.h>

#include "./lib/imageio++.h"

/* Helper function: returns true if pixel is whitish */
inline bool isWhite(Color &pixel)
{
  if (pixel.r > 240 && pixel.g > 240 && pixel.b > 240) {
    return true;
  }

  return false;
}

/* Helper function: return a [N, E, S, W] array of int arrays containg a pair of values:
 * The first index denotes the "status" of the pixel q in the corresponding
 * cardinal direction to p, and the second index will be q's index (potentially
 * out of bounds). The status type are as follows:
*    1 -> in Omega
*    0 -> in boundary of Omega
*   -1 -> not in image
*
* NB: p must be valid, as there is no boundary checking
*/
inline std::vector< std::vector<int> > getNeighbors(int p, int w, int h, ::std::vector<int>* toOmega)
{
  std::vector< std::vector<int> > results (4, std::vector<int>(3, 0));
  int q;

  // North
  q = p - w;
  results[0][1] = q;
  if (q >= 0) {
    if ((*toOmega)[q] != -1) {
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
    if ((*toOmega)[q] != -1) {
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
    if ((*toOmega)[q] != -1) {
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
    if ((*toOmega)[q] != -1) {
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

/* Helper function: returns the guidance v between p and q
 * Rmk: computed as g(p) - g(q)
 * NB: Does not check boundaries
 */
inline double guidance(Im* src, int p, int q, int channel)
{
  return (((double) (*src)[p][channel]) - ((double) (*src)[q][channel]));
}

/* Helper function: solve a sparse linear system of equations of form Ax = b
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
    //residual = gsl_splinalg_itersolve_normr(work);
    //fprintf(stderr, "iter %zu residual = %.12e\n", iter, residual);

    if (status == GSL_SUCCESS)
      fprintf(stderr, "Converged\n");
  } while (status == GSL_CONTINUE && ++iter < max_iter);

  gsl_splinalg_itersolve_free(work);
  return status;
}

/* Helper function: set channel of pixel p in dest to value v */
inline void setPixel(Im* dest, int p, double v, int channel) {
  // Clamp
  if (v > 255) {
    v = 255;
  } else if (v < 0){
    v = 0;
  }

  (*dest)[p][channel] = (unsigned char) v;
  return;
}

/* Main function */
int main(int argc, char *argv[])
{
	if (argc < 5) {
		fprintf(stderr, "Usage: %s src.png dest.png mask.png out.png\n", argv[0]);
		exit(1);
	}
	const char *srcfilename = argv[1];
	const char *destfilename = argv[2];
  const char *maskfilename = argv[3];
	const char *outfilename = argv[4];

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

  // Number of pixels in dest and mask
  int W = dest.w();
  int H = dest.h();
  int N = W*H;

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
  // RHS
  gsl_vector *r = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known reds" */
  gsl_vector *g = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known greens" */
  gsl_vector *b = gsl_vector_alloc(OMEGA_SIZE);  /* vector of "known blues" */

  /* Sparse matrix of coefficients (LHS) */
  gsl_spmatrix *A = gsl_spmatrix_alloc(OMEGA_SIZE, OMEGA_SIZE);
  gsl_vector *x = gsl_vector_alloc(OMEGA_SIZE);  /* vector for solutions (LHS) */

  /* Iterate through the pixels in Omega... */
  for (int id = OMEGA_SIZE - 1; id >= 0; id--) {
    int p = toMask[id]; // Pixel index in dest and mask
    int Np = 0;         // Number of cardinal neighbors in image
    double r_val = 0.0; // RHS of equation
    double g_val = 0.0; // RHS of equation
    double b_val = 0.0; // RHS of equation

    std::vector< std::vector<int> > neighbors = getNeighbors(p, W, H, &toOmega);

    // For each neighbor q....
    for (int j = 0; j < 4; j++) {
      int status = neighbors[j][0];
      int q = neighbors[j][1];

      // Ignore pixels outside the image
      if (status == -1) {
        continue;
      }

      Np++; // Count the neighbor
      r_val += guidance(&src, p, q, 0); // Guidance constraint
      g_val += guidance(&src, p, q, 1); // Guidance constraint
      b_val += guidance(&src, p, q, 2); // Guidance constraint

      // For q in Omega
      if (status == 1) {
        // -fq component
        int q_id = toOmega[q];
        gsl_spmatrix_set(A, id, q_id, -1.0);
      }
      // For q in boundary of Omega
      if (status == 0) {
        // f* boundary constraint
        r_val += (double) dest[q][0];
        g_val += (double) dest[q][1];
        b_val += (double) dest[q][2];
      }
    }

    // Np*fp component
    gsl_spmatrix_set(A, id, id, (double) Np);
    // Record constraints
    gsl_vector_set(r, id, r_val);
    gsl_vector_set(g, id, g_val);
    gsl_vector_set(b, id, b_val);
  }

  /* Sparsely solve the systems of equations for each channel */
  for (int c = 0; c < 3; c++) {
    if (c == 0) solve(A, x, r, OMEGA_SIZE);
    else if (c == 1) solve(A, x, g, OMEGA_SIZE);
    else if (c == 2) solve(A, x, b, OMEGA_SIZE);

    /* Copy into dest */
    for (int j = OMEGA_SIZE - 1; j >= 0; j--) {
      setPixel(&dest, toMask[j], gsl_vector_get(x, j), c);
    }
  }

  /* Free mem */
  gsl_spmatrix_free(A);
  gsl_vector_free(x);
  gsl_vector_free(r);
  gsl_vector_free(g);
  gsl_vector_free(b);

	/* Write image back out */
	if (!dest.write(outfilename))
		exit(1);

	exit(0);
}
