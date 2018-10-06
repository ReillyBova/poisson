#ifndef IMAGEIO_H
#define IMAGEIO_H
/*
Szymon Rusinkiewicz
Princeton University

imageio++.h
Simple wrapper for JPEG and PNG libraries for image input and output.
*/

#include <string>
#include <vector>


// Structure defining color at a pixel: R, G, and B as unsigned char,
// with array access also available.
struct Color {
	unsigned char r, g, b;
	const unsigned char &operator [] (int i) const
		{ return (&r)[i]; }
	unsigned char &operator [] (int i)
		{ return (&r)[i]; }
};


// Class defining an image: width, height, and pixels
class Im {
public:
	// Constructors
	Im() : width(0), height(0)
		{}
	Im(int width_, int height_) : width(width_), height(height_)
		{ pixels.resize(width * height); }

	// Accessors for width and height
	int w() const
		{ return width; }
	int h() const
		{ return height; }

	// Array access.  *No* bounds checking.
	const Color &operator [] (int i) const
		{ return pixels[i]; }
	Color &operator [] (int i)
		{ return pixels[i]; }

	// Access by (x,y) coordinate.  *No* bounds checking.
	const Color &operator () (int x, int y) const
		{ return pixels[x + y * w()]; }
	Color &operator () (int x, int y)
		{ return pixels[x + y * w()]; }

	// Read an Im from a file.  Returns true if succeeded, else false.
	bool read(const ::std::string &filename);

	// Write an Im to a file.  Returns true if succeeded, else false.
	bool write(const ::std::string &filename);

private:
	int width, height;
	::std::vector<Color> pixels;

};

#endif
