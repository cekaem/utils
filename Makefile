include Makefile.conf

all: tests

dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

tests: dirs $(BIN_DIR)/socket_test $(BIN_DIR)/timer_test

$(BIN_DIR)/socket_test: $(OBJ_DIR)/socket_test.o $(OBJ_DIR)/Socket.o Socket.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/socket_test $(OBJ_DIR)/socket_test.o $(OBJ_DIR)/Socket.o

$(OBJ_DIR)/Socket.o: Socket.cc Socket.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Socket.o Socket.cc

$(OBJ_DIR)/socket_test.o: socket_test.cc Socket.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/socket_test.o socket_test.cc

$(BIN_DIR)/timer_test: timer_test.cc Timer.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/timer_test timer_test.cc -lpthread

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
