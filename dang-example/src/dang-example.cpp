#include "pch.h"

#include "dang-gl/Window.h"

int main()
{
    dgl::WindowInfo windowInfo;
    windowInfo.size = { 1280, 720 };
    windowInfo.title = u8"Hello World!";

    dgl::Window window(windowInfo);
    window.run();
}
