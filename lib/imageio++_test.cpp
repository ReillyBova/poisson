/*
Szymon Rusinkiewicz
Princeton University

imageio++_test.cpp
Simple test of the imageio++ functionality
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "imageio++.h"


/* Helper function: returns distance from (x,y) to center of an image of
   size (w,h), scaled such that corners are 1 */
inline float dist2cent(int x, int y, int w, int h)
{
	float dx = (0.5f * w) - (x + 0.5f);
	float dy = (0.5f * h) - (y + 0.5f);
	float cx = (0.5f * w) - 0.5f;
	float cy = (0.5f * h) - 0.5f;
	return std::sqrt((dx*dx + dy*dy) / (cx*cx + cy*cy));
}


/* Main function */
int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s in.png out.png\n", argv[0]);
		exit(1);
	}
	const char *infilename = argv[1];
	const char *outfilename = argv[2];

	/* Read an image */
	Im im;
	if (!im.read(infilename))
		exit(1);

	printf("Read an image of size %d x %d\n", im.w(), im.h());

	/* Transform image - apply a vignetting filter */
	for (int y = 0; y < im.h(); y++) {
		for (int x = 0; x < im.w(); x++) {
			Color &pixel = im(x, y);
			float scale = 1.0f - dist2cent(x, y, im.w(), im.h());
			for (int j = 0; j < 3; j++)
				pixel[j] = (unsigned char) (scale * pixel[j]);
		}
	}

	/* Write image back out */
	if (!im.write(outfilename))
		exit(1);

	exit(0);
}
