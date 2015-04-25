CC = gcc

OBJ = src/main.o

CFLAGS = -Isrc/ -O3 -std=gnu99 -g

SDLOBJ = platforms/sdl/sdl.o

SDLFLAGS = -lm -lSDL -Iplatforms/sdl

dash-sdl: $(OBJ) $(SDLOBJ) Makefile
	@echo "LD $@"
	@$(CC) $(OBJ) $(SDLOBJ) -o $@ $(CFLAGS) $(SDLFLAGS) $(EXTRAFLAGS)

%.o: %.c Makefile
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(SDLFLAGS) -c $< -o $@ $(EXTRAFLAGS)

clean:
	@rm -f $(OBJ) $(SDLOBJ)
