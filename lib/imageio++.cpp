/*
Szymon Rusinkiewicz
Princeton University

imageio++.cpp
Simple wrapper for JPEG and PNG libraries for image input and output.
*/

#include <cstdio>
#include <cstring>
#include <jpeglib.h>
#include <png.h>
#include "imageio++.h"

#ifdef WIN32
# ifndef strcasecmp
#  define strcasecmp stricmp
# endif
#endif


// Read an Im from a file.  Returns true if succeeded, else false.
bool Im::read(const ::std::string &filename)
{
	using namespace std;

	FILE *f = !strcmp(filename.c_str(), "-") ? stdin :
		fopen(filename.c_str(), "rb");
	if (!f) {
		fprintf(stderr, "Couldn't open file %s\n", filename.c_str());
		return false;
	}

	/* Peek at the first two characters of the file */
	int c1 = fgetc(f);
	int c2 = fgetc(f);
	ungetc(c2, f);
	ungetc(c1, f);

	if (c1 == 0xff && c2 == 0xd8) {
		/* JPEG file */
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, f);
		jpeg_read_header(&cinfo, TRUE);
		cinfo.out_color_space = JCS_RGB;
		jpeg_start_decompress(&cinfo);
		width = cinfo.output_width;
		height = cinfo.output_height;
		pixels.resize(w() * h());
		for (int i = 0; i < h(); i++) {
			JSAMPROW rowptr = (JSAMPROW) &pixels[i * w()];
			jpeg_read_scanlines(&cinfo, &rowptr, 1);
		}
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	} else if (c1 == 0x89 && c2 == 'P') {
		/* PNG file */
		png_structp png_ptr =
			png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		png_infop end_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, f);
		png_read_info(png_ptr, info_ptr);
		png_set_expand(png_ptr);
		png_set_strip_alpha(png_ptr);
		png_set_strip_16(png_ptr);
		png_set_gray_to_rgb(png_ptr);
		width = png_get_image_width(png_ptr, info_ptr);
		height = png_get_image_height(png_ptr, info_ptr);
		pixels.resize(w() * h());
		for (int i = 0; i < h(); i++) {
			png_bytep row_pointer = (png_bytep) &pixels[i * w()];
			png_read_row(png_ptr, row_pointer, NULL);
		}
		png_read_end(png_ptr, end_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	} else {
		/* Not JPEG or PNG */
		fprintf(stderr, "Unsupported file type in %s\n", filename.c_str());
		return false;
	}
	fclose(f);
	return true;
}


/* Does string s1 end with s2?  (Case-insensitive) */
static inline bool ends_with(const char *s1, const char *s2)
{
	using namespace std;
	size_t l1 = strlen(s1), l2 = strlen(s2);
	return (l1 >= l2) && !strncasecmp(s1 + l1 - l2, s2, l2);
}


// Write an Im to a file.  Returns true if succeeded, else false.
bool Im::write(const ::std::string &filename)
{
	using namespace std;

	FILE *f = !strcmp(filename.c_str(), "-") ? stdout :
		fopen(filename.c_str(), "wb");
	if (!f) {
		fprintf(stderr, "Couldn't open file %s\n", filename.c_str());
		return false;
	}

	if (ends_with(filename.c_str(), ".jpg") ||
	    ends_with(filename.c_str(), ".jpeg")) {
		/* Write JPEG */
		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);
		cinfo.image_width = w();
		cinfo.image_height = h();
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 90, TRUE);
		cinfo.optimize_coding = TRUE;
		cinfo.dct_method = JDCT_FLOAT;
		cinfo.comp_info[0].h_samp_factor = 1;
		cinfo.comp_info[0].v_samp_factor = 1;
		jpeg_stdio_dest(&cinfo, f);
		jpeg_start_compress(&cinfo, TRUE);
		for (int i = 0; i < h(); i++) {
			JSAMPROW rowptr = (JSAMPROW) &pixels[i * w()];
			jpeg_write_scanlines(&cinfo, &rowptr, 1);
		}
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
	} else {
		/* Write PNG */
		png_structp png_ptr =
			png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, f);
		png_set_IHDR(png_ptr, info_ptr, w(), h(), 8,
			PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		for (int i = 0; i < h(); i++) {
			png_bytep row_pointer = (png_bytep) &pixels[i * w()];
			png_write_row(png_ptr, row_pointer);
		}
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
	fclose(f);
	return true;
}
