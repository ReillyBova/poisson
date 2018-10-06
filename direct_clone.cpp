/*
Reilly Bova '20
COS 526: Assignment 1

direct_clone.cpp
Directly clones src into dest based on mask (non seamless)
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "./lib/imageio++.h"

/* Helper function: returns true if pixel is white */
inline bool isWhite(Color &pixel, int x, int y)
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

	/* Transform image - apply a vignetting filter */
  /* Assumption: images are of same {W X H} */
	for (int y = dest.h() - 1; y >= 0; y--) {
		for (int x = dest.h() - 1; x >= 0; x--) {
      // Copy src onto dest if mask is white
			if (isWhite(mask(x, y), x,y)) {
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
