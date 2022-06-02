#include "dang-example/global.h"
#include "dang-glfw/GLFW.h"
#include "dang-glfw/Window.h"

int main()
{
    dglfw::GLFW glfw;

    dglfw::WindowInfo window_info;
    window_info.title = u8"Hello World!";

    dglfw::Window window(window_info);
    window.run();
}
