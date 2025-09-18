FLAGS = -Wall -Wpedantic -g
INCLUDE = -Iinclude -Iimgui -Iimgui/backends
LIBS = -lglfw -lOpenGL -lX11

main: build/renderer build/imgui
	g++ build/*.o ${LIBS} -o main

build/renderer: src/main.cpp src/glad.c src/shader.cpp 
	mkdir -p build
	g++ ${FLAGS} -c src/main.cpp ${INCLUDE} -o build/main.o
	g++ ${FLAGS} -c src/shader.cpp ${INCLUDE} -o build/shader.o
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

install: main
	cp main ${out}/
