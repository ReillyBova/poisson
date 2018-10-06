CPPFLAGS = -O
LDLIBS = -lm -ljpeg -lpng

all: direct_clone
clean:
	rm -f direct_close *.o

direct_clone: direct_clone.o ./lib/imageio++.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
direct_clone.o: ./lib/imageio++.h
imageio++.o: ./lib/imageio++.h
