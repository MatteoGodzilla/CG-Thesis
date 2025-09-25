FLAGS = -Wall -Wpedantic -g -std=c++20 
INCLUDE = -Iinclude -Iimgui -Iimgui/backends -Iglm
LIBS = -lglfw -lOpenGL -lX11

main: build/renderer build/imgui
	g++ build/*.o ${LIBS} -o main

RENDER_SRC = src/main.cpp src/glad.c src/shader.cpp src/ui.cpp src/raytracer.cpp src/framebuffer.cpp src/planet.cpp
RENDER_INC = include/shader.h include/ui.h include/raytracer.h include/framebuffer.h include/planet.h
build/renderer: ${RENDER_SRC} ${RENDER_INC}
	mkdir -p build
	g++ ${FLAGS} -c src/main.cpp ${INCLUDE} -o build/main.o
	g++ ${FLAGS} -c src/shader.cpp ${INCLUDE} -o build/shader.o
	g++ ${FLAGS} -c src/ui.cpp ${INCLUDE} -o build/ui.o
	g++ ${FLAGS} -c src/raytracer.cpp ${INCLUDE} -o build/raytracer.o
	g++ ${FLAGS} -c src/framebuffer.cpp ${INCLUDE} -o build/framebuffer.o
	g++ ${FLAGS} -c src/planet.cpp ${INCLUDE} -o build/planet.o
	g++ ${FLAGS} -c src/glad.c ${INCLUDE} -o build/glad.o
	touch build/renderer

build/imgui: imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_glfw.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui.cpp ${INCLUDE} -o build/imgui.o
	g++ ${FLAGS} -c imgui/imgui_demo.cpp ${INCLUDE} -o build/imgui_demo.o
	g++ ${FLAGS} -c imgui/imgui_draw.cpp ${INCLUDE} -o build/imgui_draw.o
	g++ ${FLAGS} -c imgui/imgui_tables.cpp ${INCLUDE} -o build/imgui_tables.o
	g++ ${FLAGS} -c imgui/imgui_widgets.cpp ${INCLUDE} -o build/imgui_widgets.o
	g++ ${FLAGS} -c imgui/imgui_widgets.cpp ${INCLUDE} -o build/imgui_widgets.o
	g++ ${FLAGS} -c imgui/backends/imgui_impl_opengl3.cpp ${INCLUDE} -o build/imgui_impl_opengl3.o
	g++ ${FLAGS} -c imgui/backends/imgui_impl_glfw.cpp ${INCLUDE} -o build/imgui_impl_glfw.o
	touch build/imgui

clean: 
	rm -r build/

install: main
	cp main ${out}/
