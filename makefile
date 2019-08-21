CXX = g++
CXXFLAGS = -std=c++11
LDLIBS = -lwiringPi

HEADERS = 
OBJECTS = ice40.o 

default: ice40

ice40: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

%.o: ./%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(OBJECTS)
	-rm -f ice40