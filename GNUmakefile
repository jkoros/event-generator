include config.gmk

OBJS = diff2poly interpolate fluxWeight rndmSample

diff2poly: diff2poly.o
	LD_RUN_PATH= $(CXX) $(CXXFLAGS) diff2poly.o -o diff2poly $(LDLIBS)

interpolate: interpolate.o
	LD_RUN_PATH= $(CXX) $(CXXFLAGS) interpolate.o -o interpolate $(LDLIBS)

fluxWeight: fluxWeight.o
	LD_RUN_PATH= $(CXX) $(CXXFLAGS) fluxWeight.o -o fluxWeight $(LDLIBS)

rndmSample: rndmSample.o
	LD_RUN_PATH= $(CXX) $(CXXFLAGS) rndmSample.o -o rndmSample $(LDLIBS)

all: $(OBJS)

clean:
	$(RM) -r *.o *~ $(OBJS)