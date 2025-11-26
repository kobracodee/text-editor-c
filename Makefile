#CFLAGS = -std=c17 -Wall -Wextra -Werror
#
#INCLUDES = -IC:/Libraries/SDL2/include/SDL2 -IC:/Libraries/SDL2/include -IC:/Libraries/SDL2_ttf/include
#
#LIBPATH = -LC:/Libraries/SDL2/lib -LC:/Libraries/SDL2_ttf/lib
#
#LIBS = -lmingw32 -lSDL2 -lSDL2_ttf
#
#SRC = main.c
#EXE = text-editor.exe
#
#all:
#	gcc $(SRC) -o $(EXE) $(CFLAGS) $(INCLUDES) $(LIBPATH) $(LIBS)
#
#rm:
#	-del -fR $(EXE)

CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Werror
INCLUDES = -Iinclude -IC:/Libraries/SDL2/include -IC:/Libraries/SDL2/include/SDL2 -IC:/Libraries/SDL2_ttf/include
LIBPATH = -LC:/Libraries/SDL2/lib -LC:/Libraries/SDL2_ttf/lib
LIBS = -lmingw32 -lSDL2 -lSDL2_ttf

SRC_DIR = src
OBJ_DIR = obj

EXE = text-editor.exe

# Collect all source files
SRC = main.c $(wildcard $(SRC_DIR)/*.c)

# Generate obj/main.o and obj/foo.o from main.c + src/foo.c
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

all: $(EXE)

# Link
$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBPATH) $(LIBS)

# Compile each .c file into obj/<name>.o
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create obj directory if missing
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	-del /Q $(OBJ_DIR)\*.o 2>NUL
	-del /Q $(EXE) 2>NUL

