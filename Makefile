CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)

INC = $(shell pwd)

CPPFLAGS := $(shell root-config --cflags) -I$(INC)/include
#LDFLAGS := $(LDFLAGS) $(shell root-config --glibs) -lMathMore
LDFLAGS := $(LDFLAGS) $(shell root-config --glibs)
CPPFLAGS += -g

TARGET  = ac_lgad

SRC = app/main.cc src/pulse.cc src/CommandLineInput.cc

OBJ = $(SRC:.cc=.o)

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(LD) $(CPPFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
	@echo $@
	@echo $<
	@echo $^

%.o : %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<
	@echo $@
	@echo $<

clean :
	rm -f *.o src/*.o $(TARGET) app/*.o include/*.o *~
