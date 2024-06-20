#!/usr/bin/bash

C_FILES="instanced_quads.c glad.c"

# gcc -std=c99 -g -O0 $C_FILES -o exe -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

gcc -std=c99 -Werror -Wall -Wextra -Wno-unused-parameter $C_FILES -g -lX11 -pthread -lm -ldl -lpthread -lrt -lOpenGL -lglfw -o exe

