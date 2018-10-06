CPPFLAGS = -O
LDLIBS = -lm -ljpeg -lpng

all: imageio++_test
clean:
	rm -f imageio++_test *.o

imageio++_test: imageio++_test.o imageio++.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
imageio++_test.o: imageio++.h
imageio++.o: imageio++.h
