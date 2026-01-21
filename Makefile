# PHONEX SYSTEMS // MAKEFILE
# TARGET: Linux / MacOS / WSL
# COMPILER: gcc (C17)

CC = gcc
CFLAGS = -Wall -Werror -std=c17 -O2 -D_XOPEN_SOURCE=700
LIBS = -lm

SRC_DIR = src
OBJ_DIR = build

# Source Files
SRCS = $(SRC_DIR)/core/main.c \
       $(SRC_DIR)/core/accounting.c \
       $(SRC_DIR)/fin/market_gen.c \
       $(SRC_DIR)/ui/render.c

# Output Binary
TARGET = phonex_am

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(OBJ_DIR)
	@echo "   [COMPILE] PHONEX CORE..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)
	@echo "   [BUILD]   SUCCESS. RUN ./${TARGET}"

clean:
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)

run: all
	./$(TARGET)