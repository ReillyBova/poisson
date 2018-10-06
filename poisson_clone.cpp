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
#include "./lib/imageio++.h"

/* Helper function: returns true if pixel is white */
inline bool isWhite(Color &pixel)
{
  if (pixel.r == 255 && pixel.g == 255 && pixel.b == 255) {
    return true;
  }

  return false;
}

/* Helper function: returns true if pixel is white */
inline bool xyToOneDim(int x, int y)
{
  if (pixel.r == 255 && pixel.g == 255 && pixel.b == 255) {
    return true;
  }

  return false;
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
    fprintf(stderr, "Usage: dest and mask images must have identical dimensions \n", argv[0]);
		exit(1);
  }

  // Number of pixels in dest and mask
  int N = dest.pixels.size();

  /* Map pixel indices to Omega membership, given by ID <id>. An ID of -1 implies
  *  the pixel lies outside a mask (it may still be a boundary pixel though) */
  ::std::vector<int> toOmega (N, -1);
  int id = 0;
  for (int i = N; i >= 0; i--) {
    if (isWhite(mask[i])) {
      toOmega[i] = id;
      id++;
    }
  }

  /* Now reverse the mapping now that we now how many ids we have */
  ::std::vector<int> toMask (id);
  for (int i = N; i >= 0; i--) {
    if (toOmega[i] >= 0) {
      toMask[toOmega[i]] = i;
    }
  }

	/* Transform image - apply a vignetting filter */
  /* Assumption: images are of same {W X H} */
	for (int y = dest.h() - 1; y >= 0; y--) {
		for (int x = dest.h() - 1; x >= 0; x--) {
      // Copy src onto dest if mask is white
			if (isWhite(mask(x, y))) {
        Color &src_pixel = src(x, y);
        Color &dest_pixel = dest(x, y);

  			for (int j = 0; j < 3; j++)
  				dest_pixel[j] = (unsigned char) src_pixel[j];
      }
    }
	}

	/* Write image back out */
	if (!dest.write(outfilename))
		exit(1);

	exit(0);
}
