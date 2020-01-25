#pragma once

#include "dang-math/vector.h"

namespace dang::gl
{

class WindowInfo {
public:
	WindowInfo();
								  
	dmath::ivec2 size() const;
	void setSize(dmath::ivec2 size);

	int width() const;				
	void setWidth(int width);
	
	int height() const;		 		
	void setHeight(int height);
	
	std::string title() const;
	void setTitle(std::string title);

	GLFWwindow* createWindow() const;

private:
	dmath::ivec2 size_;
	std::string title_;
};

class Window {
public:
	Window(GLFWwindow* handle);
	Window(const WindowInfo& info = WindowInfo());
	~Window();

	GLFWwindow* handle();

	bool shouldClose();

	void update();
	void render();

	void step();
	void run();

private:
	GLFWwindow* handle_;
};

}