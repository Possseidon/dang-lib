#include "pch.h"

#include "dang-gl/Window.h"

int main()
{
    dgl::WindowInfo window_info;
    window_info.title = u8"Hello World!";

    dgl::Window window(window_info);
    window.run();
}
