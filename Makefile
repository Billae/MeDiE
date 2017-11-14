include Makefile.inc

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o


EXEC=main

all: $(EXEC)

$(EXEC): c s

#client
c: $(OBJ_DIR)/client.o
	$(CC) $< -o $(BIN_DIR)/client $(CFLAGS) $(LDFLAGS)

#serveur
s: $(OBJ_DIR)/server.o
	$(CC) $< -o $(BIN_DIR)/server $(CFLAGS) $(LDFLAGS)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)



.PHONY: all

clean:
	@rm -f $(OBJ_DIR)/*.o

mrproper: clean
	@rm -f $(BIN_DIR)/*
