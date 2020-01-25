#include "pch.h"

#include "dang-gl/Window.h"

int main()
{
	dgl::WindowInfo windowInfo;
	windowInfo.setSize({1280, 720});
	windowInfo.setTitle(u8"Hello World!");

	dgl::Window window(windowInfo);
	window.run();
}
