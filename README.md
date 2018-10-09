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

First, install the prerequisites using your favorite method (`homebrew` is recommended for Mac OSX). Then, download this repository, and run the Makefile using`$ make`. The C++ program should compile into `poisson_clone` without errors.

### Running the Program

Once `poisson_clone` has been compiled, run it in the command line using the following arguments

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
  * no flag or unrecognized flag => seamless poisson cloning
  * "-d" or "-direct" => direct cloning
  * "-mono" or "-monochrome" => convert src to monochrome before applying poisson cloning
  * "-mx" or "-mixed" => use mixed cloning (mix gradients of dest and src)
  * "-f" or "-flat" followed by `threshold factor` => Threshold gradients above `threshold` and scale them by `factor`
  * "-il" or "-illumination" followed by `alpha beta` => Apply exposure/specular correction using `alpha` and `beta` as parameters for applying a nonlinear transformation to the source gradient
  * "-dec" or "-decolor" => Attempt to decolor the background by converting the destination to monochrome before applying seamless Poisson cloning
  * "-rec" or "-recolor" followed by `scaleR scaleG scaleB` => Scale color source channels by the provided parameters before applying poisson cloning
  * "-tex" or "-texture" followed by `threshold` => Preserve grain (gradient below threshold) in dest

## Cloning Modes & Examples

This section contains descriptions of each cloning mode in this program, along with examples on how to run them.

### Poisson Cloning
Poisson cloning is the main workhorse of this program and is run without flags:

```
$ ./poisson_clone src.png mask.png dest.png out.png xOffset yOffset [-FLAG [extraArgs]]
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

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

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
    * no flag or unrecognized flag => seamless poisson cloning
    * "-d" or "-direct" => direct cloning
    * "-mono" or "-monochrome" => convert src to monochrome before applying poisson cloning
    * "-mx" or "-mixed" => use mixed cloning (mix gradients of dest and src)

Tip: To line up the offsets of your source image within the mask, run with the
    direct cloning flag ("-d" or "-direct") enabled. Then, once you know the
    proper offset for cloning, remove the flag to run poisson cloning.
