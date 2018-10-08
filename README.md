# poisson
A C++ program that implements Poisson image-composition as described by Perez et al. in 2003

Installation: $ make

Dependancies: gsl, libjpeg, libpng, and gsl (recommended: use homebrew to install)

Usage: '$./poisson_clone src.png mask.png dest.png out.png xOffset yOffset [(-d || -direct)]'
    ./poisson_clone is run with the following arguments: the following arguments:
    
      * src.png => file path to source image (required)
      * mask.png => file path to mask image (required)
      * dest.png => file path to dest image; must be of same dim as mask.png (required)
      * out.png => file path to write result (required)
      * xOffset => x offset in src.png (required)
      * yOffset => y offset in src.png (required)
      * flag argument (optional):
        * no flag or unrecognized flag => seamless poisson cloning
        * "-d" or "-direct" => direct cloning

Tip: To line up the offsets of your source image within the mask, run with the
    direct cloning flag ("-d" or "-direct") enabled. Then, once you know the
    proper offset for cloning, remove the flag to run poisson cloning.
