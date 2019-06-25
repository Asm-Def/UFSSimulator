cppc = g++
cppflags = -O2 -std=c++11
SRC = ./UFS/src
INC = ./UFS/include
BIN = ./UFS/bin

all : test
test : $(BIN)/test

$(BIN)/test: $(BIN) $(INC)/UFSParams.h $(INC)/SuperBlock.h $(INC)/VHDController.h $(SRC)/VHDController.cpp $(SRC)/test.cpp
	$(cppc) $(cppflags) -g -DDEBUG $(SRC)/test.cpp $(SRC)/VHDController.cpp -o $(BIN)/test

$(BIN):
	mkdir -p $@

clean:
	rm -f $(BIN)/*