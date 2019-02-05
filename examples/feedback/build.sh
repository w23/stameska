cc -O3 -DSYNC_PLAYER $(pkg-config --cflags --libs sdl) -I../../3p/rocket/lib -lGL intro.c ../../3p/rocket/lib/*.c -lm -o feedback && ./feedback
