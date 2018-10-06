CPPFLAGS = -O
LDLIBS = -lm -ljpeg -lpng -lgsl

all: poisson_clone
clean:
	rm -f poisson_clone *.o

poisson_clone: poisson_clone.o ./lib/imageio++.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
poisson_clone.o: ./lib/imageio++.h
imageio++.o: ./lib/imageio++.h
