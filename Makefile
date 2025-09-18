FLAGS = -Wall -Wpedantic -g
INCLUDE = -Iinclude
LIBS = -lglfw -lOpenGL

main: src/main.cpp src/glad.c src/shader.cpp
	g++ ${FLAGS} src/main.cpp src/shader.cpp src/glad.c ${INCLUDE} ${LIBS} -o main

install: main
	cp main ${out}/
