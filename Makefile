cppc = g++
cppflags = -O2 -std=c++11
SRC = ./UFS/src
INC = ./UFS/include
BIN = ./UFS/bin

all : test
test : $(BIN)/test
os : $(BIN)/os

$(BIN)/os: $(BIN) $(INC)/*.h $(SRC)/TestOS.cpp
	$(cppc) $(cppflags) -g -DDEBUG $(SRC)/TestOS.cpp $(SRC)/FileSystem.cpp $(SRC)/INode.cpp $(SRC)/OpenedFile.cpp $(SRC)/ProcessFDTable.cpp $(SRC)/UserManager.cpp $(SRC)/VHDController.cpp -o $(BIN)/os

$(BIN)/test: $(BIN) $(INC)/*.h $(SRC)/VHDController.cpp $(SRC)/FileSystem.cpp $(SRC)/OpenedFile.cpp $(SRC)/test.cpp
	$(cppc) $(cppflags) -g -DDEBUG $(SRC)/test.cpp $(SRC)/VHDController.cpp $(SRC)/FileSystem.cpp $(SRC)/INode.cpp -o $(BIN)/test

$(BIN):
	mkdir -p $@

clean:
	rm -f $(BIN)/*
