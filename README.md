# Poisson Image Editing

This repository contains a C++ program that implements the seamless Poisson image-cloning technique described in ["Poisson Image Editing"](http://www.cs.princeton.edu/courses/archive/fall18/cos526/papers/perez03.pdf) by Patrick Pérez, Michel Gangnet, & Andrew Blake in 2003.

## Getting Started

Follow these instructions in order to run this program on your local machine (NB: this has only been tested on Mac OSX).

### Prerequisites

This project requires the following libraries:
* [GNU Science Library](https://www.gnu.org/software/gsl/)
* [libjpeg](https://www.ijg.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)

### Installing

First, install the prerequisites using your favorite method ([homebrew](https://brew.sh/) is recommended for Mac OSX). Then, download this repository, and run the Makefile using `$ make`. The C++ program should compile into `poisson_clone` without errors.

### Running the Program

Once `poisson_clone` has been compiled, run it in the command line using the following arguments:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset [-FLAG [extraArgs]]
```

Here is a breakdown of the meaning of the arguments and avaliable flags:
* src.png => file path to source image (required)
* mask.png => file path to mask image (required)
* dest.png => file path to dest image; must be of same dim as mask.png (required)
* out.png => file path to write result (required)
* xOffset => x offset in src.png (required)
* yOffset => y offset in src.png (required)
* flag argument (optional):
  * no flag or unrecognized flag => seamless Poisson cloning
  * "-d" or "-direct" => direct cloning
  * "-mono" or "-monochrome" => convert src to monochrome before applying Poisson cloning
  * "-mx" or "-mixed" => use mixed cloning (mix gradients of dest and src)
  * "-f" or "-flat" followed by `threshold factor` => Threshold gradients above `threshold` and scale them by `factor`
  * "-il" or "-illumination" followed by `alpha beta` => Apply exposure/specular correction using `alpha` and `beta` as parameters for applying a nonlinear transformation to the source gradient
  * "-dec" or "-decolor" => Attempt to decolor the background by converting the destination to monochrome before applying seamless Poisson cloning
  * "-rec" or "-recolor" followed by `scaleR scaleG scaleB` => Scale color source channels by the provided parameters before applying Poisson cloning
  * "-tex" or "-texture" followed by `threshold` => Preserve grain (gradient below threshold) in dest

## Cloning Modes & Examples

This section contains descriptions of each cloning mode in this program, along with examples on how to run them.

### Poisson Cloning

#### Usage
Poisson cloning is the main workhorse of this program and is run without flags:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset
```

#### Explanation
This method of cloning solves a sparse linear system of equations of the form `Ax = b` bound by two contraints: (1) the border of the cloned region must match the border of the region before cloning, and (2) the color gradient-field within the pasted cloned-region must match the gradient-field of the source cloned-region as closely as possible.

In pseudocode, the matrices and vectors for this system of equations are constructed as follows:
```
for each pixel i in the mask:
  Np = 0  // "Number of neighbors"
  for each neighbor j of i:
    set Aij to -1 if j in mask;
    add dest[j] to b_i if j in image but not in mask (=> j is a border pixel)
    add 1 to Np if j is in image
    add guidance (src[i] - src[j]) to b_i if j is in image
  set Aii to num neighbors Np
```

#### Results

|  Direct Cloning | Poisson Cloning | 
|:--------------:|:----------------:|
| ![Direct Cloning](/results/battleOfPrinceton_direct.png?raw=true) | ![Poisson Cloning](/results/battleOfPrinceton_poisson.png?raw=true) |
| ![Direct Cloning](/results/fig3a_direct.png?raw=true) | ![Poisson Cloning](/results/fig3a_poisson.png?raw=true) |
| ![Direct Cloning](/results/fig3b_direct.png?raw=true) | ![Poisson Cloning](/results/fig3b_poisson.png?raw=true) |
| ![Direct Cloning](/results/fig4_direct.png?raw=true) | ![Poisson Cloning](/results/fig4_poisson.png?raw=true) |


### Direct Cloning
#### Usage
Direct cloning is the naive (seamed) implementation of cloning and requires a `-d` or `-direct` flag. It takes no further parameters:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -d
```
or
```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -direct
```


### Monochromatic Poisson Cloning
#### Usage
Monochromatic cloning requires the `-mono` or `-monochrome` flag and takes no further parameters:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -mono
```
or
```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -monochrome
```

#### Explanation
First monochromatic cloning converts the source image to greyscale through luminance, and it then applies Poisson cloning on using the black and white source image. This is useful when the chromacity of the cloned region needs to remain relatively constant, as the default (polychromatic) Poisson cloning will allow for changes in color within the cloned region that are somewhat independent of the destination's border constraints.

#### Results

| Source | Destination | Polychromatic Poisson Cloning | Monochromatic Poisson Cloning | 
|:--------------:|:----------------:|:----------------:|:----------------:|
| ![Source](/test_images/perez-fig5-src.png?raw=true) | ![Destination](/test_images/perez-fig5-dst.png?raw=true) |![Poisson Cloning](/results/fig5_poisson.png?raw=true) | ![Monochromatic Cloning](/results/fig5_mono.png?raw=true) |


### Mixed Poisson Cloning
#### Usage
Mixed cloning requires the `-mx` or `-mixed` flag and takes no further parameters:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -mx
```
or
```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset -mixed
```

#### Explanation
Sometimes there is detail in the target region of the destination that needs to be preserved (e.g. a brick wall, or a sharp edge in the background). This is accomplished by adjusting the guidance function between two pixels (i, j) such that if the gradient between pixels i and j are greater (by magnitude) in the destitiation image than in the source, this larger gradient will be used for the system of equations

#### Results

| Source | Destination | Poisson Cloning | Mixed Poisson Cloning | 
|:--------------:|:----------------:|:----------------:|:----------------:|
| ![Source](/test_images/perez-fig6-src.png?raw=true) | ![Destination](/test_images/perez-fig6-dst.png?raw=true) |![Poisson Cloning](/results/fig6_poisson.png?raw=true) | ![Mixed Cloning](/results/fig6_mixed.png?raw=true) |
| ![Source](/test_images/perez-fig7-src.png?raw=true) | ![Destination](/test_images/perez-fig7-dst.png?raw=true) |![Poisson Cloning](/results/fig7_poisson.png?raw=true) | ![Mixed Cloning](/results/fig7_mixed.png?raw=true) |
| ![Source](/test_images/perez-fig8-src.png?raw=true) | ![Destination](/test_images/perez-fig8-dst.png?raw=true) |![Poisson Cloning](/results/fig8_poisson.png?raw=true) | ![Mixed Cloning](/results/fig8_mixed.png?raw=true) |


### Image Flattening
#### Usage
Image flattening requires the `-f` or `-flat` flag along with `threshold` and `factor` values. It is recommended that the source and destination arguments point to the same image, and that the offsets are set to `0`:

```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -f threshold factor
```
or
```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 - flat threshold factor
```

#### Explanation
Image flattening throws away gradients in the source that do not exceed the threshold value in magnitude. As such, only the sharper edges and features of the image are preserved, giving a flattening feel. The preserved gradients are either compressed or heightened depending on whether `factor` is greater than or less than `1` respectively.

#### Results

| Source | Image Flattening |
|:--------------:|:----------------:|
| ![Source](/test_images/perez-fig9-src.png?raw=true) | ![Flat](/results/fig9_flat.png?raw=true) |


### Local Illumination Changes
#### Usage
Local illumination requires the `-il` or `-illumination` flag along with `alpha` and `beta` values. It is recommended that the source and destination arguments point to the same image, and that the offsets are set to `0`:

```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -il alpha beta
```
or
```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -illumination alpha beta
```

#### Explanation
Nonlinear expansion and compression of the source image gradient (applied through alpha and beta) can be used to fix locally underexposed (e.g. shadows) and overexposed (e.g. specular highlights) areas in images. It is recommended that `alpha` is set to a value in the vicinity of `0.2` times the average gradient of the image, and that `beta` be set to a value simply within the neighborhood of `0.2`.

#### Results

| Source | Local Illumination |
|:--------------:|:----------------:|
| ![Source](/test_images/perez-fig10a-src.png?raw=true) | ![Illuminated](/results/fig10a_illum.png?raw=true) |
| ![Source](/test_images/perez-fig10b-src.png?raw=true) | ![Illuminated](/results/fig10b_illum.png?raw=true) |


### Background Decolorization
#### Usage
Background decolorization requires the `-dec` or `-decolor` flag and takes no further parameters. It is recommended that the source and destination arguments point to the same image, and that the offsets are set to `0`:

```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -dec
```
or
```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -decolor
```

#### Explanation
If there is a particularly colorful area within an image (more specifically, that the region has a distinct color from its immediate surroundings), then if the source is Poisson-cloned onto a greyscale version of itself, the colorful region should still retain its color through the cloning process.

#### Results

| Source | Background Decolorization |
|:--------------:|:----------------:|
| ![Source](/test_images/perez-fig11-src.png?raw=true) | ![Decolorization](/results/fig11_dec.png?raw=true) |


### Local Recolorization
#### Usage
Local recolorization requires the `-rec` or `-recolor` flag as well as three additional parameters that coorespond to the scaling of RGB in recolorization. It is recommended that the source and destination arguments point to the same image, and that the offsets are set to `0`:

```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -rec scaleR scaleG scaleB
```
or
```
$ ./poisson_clone src.png mask.png src.png out.png 0 0 -recolor scaleR scaleG scaleB
```

#### Explanation
Recoloring works by scaling RGB values in the source image by the specified amounts before applying Poisson cloning. The bordering region around the object of interest in the mask will return to its original color when the boundary constraints of the cloning process are applied, the but object of interest itself will retain its new color.

#### Results

| Source | Background Local Recolorization |
|:--------------:|:----------------:|
| ![Source](/test_images/perez-fig11-src.png?raw=true) | ![Recolorization](/results/fig11_rec.png?raw=true) |


### Texture Preserving Poisson Cloning
#### Usage
Texture preserving poisson cloning requires the `-tex` or `-texture` flag as well as one addition `threshold` argument:

```
$ ./poisson_clone src.png mask.png src.png out.png xOffset yOffset -tex threshold
```
or
```
$ ./poisson_clone src.png mask.png src.png out.png xOffset yOffset -texture threshold
```

#### Explanation
This is an experimental extension that was not proposed in the original 2003 paper. Although Poisson cloning is excellent at removing seams, differences between cloned textures and the destination textures (e.g. grain) can still leave an apparent seam around the cloning region. In an effort to preserve grain and other small details of the destination, gradients in the destination below the threshold are added onto the source gradient in this mode. Unfortunately, this method is not always successful as discoloration occasionally pops up, and it is not able to remove any grain from the source (so it effectively only adds grain and other nearly imperceptible details).

#### Results

| Poisson Cloning vs. Poisson Cloning with Texture Preservation |
|:--------------:|
| ![Texture Comparison](/results/texture_comparison.png?raw=true) |


## Authors

* **Billie Thompson** - *Initial work* - [PurpleBooth](https://github.com/PurpleBooth)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc



# poisson
A C++ program that implements Poisson image-composition as described by Perez et al. in 2003

Installation: `$ make`

Dependancies: gsl, libjpeg, libpng, and gsl (recommended: use homebrew to install)

Usage: `$./poisson_clone src.png mask.png dest.png out.png xOffset yOffset [flag]`

./poisson_clone is run with the following arguments: the following arguments:

  * src.png => file path to source image (required)
  * mask.png => file path to mask image (required)
  * dest.png => file path to dest image; must be of same dim as mask.png (required)
  * out.png => file path to write result (required)
  * xOffset => x offset in src.png (required)
  * yOffset => y offset in src.png (required)
  * flag argument (optional):
    * no flag or unrecognized flag => seamless Poisson cloning
    * "-d" or "-direct" => direct cloning
    * "-mono" or "-monochrome" => convert src to monochrome before applying Poisson cloning
    * "-mx" or "-mixed" => use mixed cloning (mix gradients of dest and src)

Tip: To line up the offsets of your source image within the mask, run with the
    direct cloning flag ("-d" or "-direct") enabled. Then, once you know the
    proper offset for cloning, remove the flag to run Poisson cloning.
