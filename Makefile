SRC_DIR = src
OBJ_DIR = obj

CXX = g++
WARN = -Wall
ERR = -Werror

ifdef DEBUG
OPT = -g
DEBUG = -DDEBUG
else
OPT = -O3
DEBUG = 
endif

CXX_FLAGS = -std=c++11 $(OPT) $(WARN) $(ERR) $(INC) $(LIB) $(DEBUG) 
LD_LIBS = -lm

# check https://makefiletutorial.com/#fancy-rules for why it works 

SRC = $(wildcard $(SRC_DIR)/*.cc)
OBJ = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRC))


all: smp_cache
	@echo "Compilation Done ---> nothing else to make :) "

smp_cache: $(OBJ)
	$(CXX) $^ -o $@ $(LD_LIBS)
	@echo "------------------------------------------------------------"
	@echo "--- ECE/CSC 406/506 FALL'23 COHERENCE PROTOCOL SIMULATOR ---"
	@echo "------------------------------------------------------------"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc | $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

$(OBJ_DIR): 
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) smp_cache *.zip

PROTOCOL = 1
TRACE_FILE = traces/canneal.04t.longTrace
VALIDATION_FILE = val/Dragon_long.val

run: all
	@echo "*** Running ./smp_cache 8192 8 64 4 $(PROTOCOL) $(TRACE_FILE) ***"
	./smp_cache 8192 8 64 4 $(PROTOCOL) $(TRACE_FILE) > run.log

val: all
	@echo "*** Comparing output with $(VALIDATION_FILE) ***"
	./smp_cache 8192 8 64 4 $(PROTOCOL) $(TRACE_FILE) | diff -iwy - $(VALIDATION_FILE)

pack:
	# zip -j project2.zip *.cc *.h
	zip -j project2.zip src/*.cc src/*.h spec/ece506_project2.pdf
