/*
Primary file to change if changing window backends. This is the primary interface that connects the engine to the window.

Current backend used: SDL2
*/

#pragma once

#include <unordered_map>
#include <functional>
#include <string>

#include <SDL2-2.30.7/include/SDL.h>
#include "../Editor/Input/KeyDefines.h"

#include "../Datatypes/Vector2Int.h"

namespace gde {

    class Window {
    private:
        SDL_Window* implemented_window;

        Vector2Int mousePos;

        std::unordered_map<std::string, std::function<void(void*)>> window_callbacks;

        std::unordered_map<unsigned int, int> keystates;
    public:
        Window();

        void UpdateState();
        void SwapBuffers();
        void Terminate();

        bool ShouldClose();

        void SetContextToThis();
        void* GetNativeHandle();

        int Get_win_x();
        int Get_win_y();

        void RegisterWindowCallback(const std::string, std::function<void(void*)>);
        void InvokeWindowCallback(const std::string, void*);

        bool GetKeyState(unsigned int keyId);
        Vector2Int GetMousePos();
    };
}