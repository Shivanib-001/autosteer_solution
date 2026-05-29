# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g
# Use LDLIBS for library linking (e.g., -lm for math library)
LDLIBS = -lm -lcjson
# Directories
SRC_DIR = src
OBJ_DIR = build
BIN = $(OBJ_DIR)/main
LOGFILE = build.log

# Find all source files and convert them to .o in the build directory
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default target: compile and run
all: $(BIN)
	./$(BIN) 

# Default target: compile , run and log
#all: $(BIN)
#	./$(BIN) >> build.log 2>&1

# Link the executable
$(BIN): $(OBJS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(OBJS) -o $(BIN) $(LDLIBS) 

## Link the executable with log
#$(BIN): $(OBJS)
#	@mkdir -p $(OBJ_DIR)
#	$(CC) $(OBJS) -o $(BIN) $(LDLIBS) > $(LOGFILE) 2>&1

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR)




#all: $(TARGET)
#$(TARGET): $(OBJS) $(CC) $(CFLAGS) -o $(TARGET) $(OBJS) 
#%.o: %.c $(CC) $(CFLAGS) -c $< -o $@
#run: $(TARGET) ./$(TARGET)

#clean: rm -f $(TARGET) $(OBJS)

