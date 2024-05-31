#!/usr/bin/bash

C_FILES="main.c glad.c"

gcc -g -O0 $C_FILES -o exe -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm
