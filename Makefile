FLAGS = -Wall -Wpedantic -g -std=c++20 
INCLUDE = -Iinclude -Iglad/include -Iimgui -Iimgui/backends -Iglm -Icxxopts/include
LIBS = -lglfw -lOpenGL -lX11

main: src/main.cpp glad/src/glad.c build/renderer build/gui build/imgui 
	mkdir -p build
	g++ ${FLAGS} -c src/main.cpp ${INCLUDE} -o build/main.o
	g++ ${FLAGS} -c glad/src/glad.c ${INCLUDE} -o build/glad.o
	g++ build/*.o ${LIBS} -o main

# Meta target for compiling the renderer by itself
build/renderer: build/mainRenderer.o
	touch build/renderer

build/mainRenderer.o: src/mainRenderer.cpp include/mainRenderer.h
	g++ ${FLAGS} -c src/mainRenderer.cpp ${INCLUDE} -o build/mainRenderer.o

# Meta target for compiling the gui editor
build/gui: build/mainUI.o build/shader.o build/ui.o build/raytracer.o build/framebuffer.o build/planet.o build/flag.o build/viewport.o build/serialization.o
	touch build/gui

build/mainUI.o: src/mainUI.cpp include/mainUI.h
	mkdir -p build
	g++ ${FLAGS} -c src/mainUI.cpp ${INCLUDE} -o build/mainUI.o

build/shader.o: src/shader.cpp include/shader.h 
	mkdir -p build
	g++ ${FLAGS} -c src/shader.cpp ${INCLUDE} -o build/shader.o

build/ui.o: src/ui.cpp include/ui.h 
	mkdir -p build
	g++ ${FLAGS} -c src/ui.cpp ${INCLUDE} -o build/ui.o
	
build/raytracer.o: src/raytracer.cpp include/raytracer.h 
	mkdir -p build
	g++ ${FLAGS} -c src/raytracer.cpp ${INCLUDE} -o build/raytracer.o

build/framebuffer.o: src/framebuffer.cpp include/framebuffer.h 
	mkdir -p build
	g++ ${FLAGS} -c src/framebuffer.cpp ${INCLUDE} -o build/framebuffer.o

build/planet.o: src/planet.cpp include/planet.h 
	mkdir -p build
	g++ ${FLAGS} -c src/planet.cpp ${INCLUDE} -o build/planet.o

build/flag.o: src/flag.cpp include/flag.h 
	mkdir -p build
	g++ ${FLAGS} -c src/flag.cpp ${INCLUDE} -o build/flag.o

build/viewport.o: src/viewport.cpp include/viewport.h 
	mkdir -p build
	g++ ${FLAGS} -c src/viewport.cpp ${INCLUDE} -o build/viewport.o

build/serialization.o: src/serialization.cpp include/serialization.h 
	mkdir -p build
	g++ ${FLAGS} -c src/serialization.cpp ${INCLUDE} -o build/serialization.o

# Meta target for compiling imgui
build/imgui: build/imgui.o build/imgui_demo.o build/imgui_draw.o build/imgui_tables.o build/imgui_widgets.o build/imgui_impl_opengl3.o build/imgui_impl_glfw.o
	touch build/imgui

build/imgui.o: imgui/imgui.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui.cpp ${INCLUDE} -o build/imgui.o

build/imgui_demo.o: imgui/imgui_demo.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui_demo.cpp ${INCLUDE} -o build/imgui_demo.o

build/imgui_draw.o: imgui/imgui_draw.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui_draw.cpp ${INCLUDE} -o build/imgui_draw.o

build/imgui_tables.o: imgui/imgui_tables.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui_tables.cpp ${INCLUDE} -o build/imgui_tables.o

build/imgui_widgets.o: imgui/imgui_widgets.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/imgui_widgets.cpp ${INCLUDE} -o build/imgui_widgets.o

build/imgui_impl_opengl3.o: imgui/backends/imgui_impl_opengl3.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/backends/imgui_impl_opengl3.cpp ${INCLUDE} -o build/imgui_impl_opengl3.o

build/imgui_impl_glfw.o: imgui/backends/imgui_impl_glfw.cpp
	mkdir -p build
	g++ ${FLAGS} -c imgui/backends/imgui_impl_glfw.cpp ${INCLUDE} -o build/imgui_impl_glfw.o

clean: 
	rm -r build/

install: main
	cp main ${out}/
