
INCLUDES=-I./ -I./include/activemq-cpp-3.9.5 -I./include/apr-1 -I./include/uuid
CFLAGS = -pthread -W -Wall -Wextra -Wconversion -fPIC -fstrict-aliasing -Wstrict-aliasing=2 -Wno-long-long
CFLAGS += -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += $(INCLUDES)
LFLAGS = -L./lib/shared -lactivemq-cpp -lapr-1 -luuid -lrt -lcrypt -lpthread -ldl
OBJ_PATH=./obj
BIN=amq_mt_pool

include OBJS.def

all: compile
compile: $(OBJS)
	$(CXX) -o $(BIN) $(OBJS) $(LFLAGS)

$(OBJ_PATH)/%.o: ./%.cpp
	@if [ ! -d $(OBJ_PATH) ]; then \
		mkdir $(OBJ_PATH); \
	fi
	g++ -std=c++11 -c $(CFLAGS) -o $@ $<

rebuild: clean all

clean:
	rm -rf $(OBJ_PATH)
	rm -f $(BIN)
