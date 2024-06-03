#include "Window.h"

namespace gde {
    Window::Window(int win_x, int win_y)
    {
        this->win_x = win_x;
        this->win_y = win_y;

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(win_x, win_y, "PCO1 RAYO, GABRIEL PAULO A.", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);
    }
}