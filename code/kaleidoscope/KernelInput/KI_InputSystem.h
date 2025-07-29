//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 00:49:54
//

#pragma once

#include <glm/glm.hpp>
#include <KernelCore/KC_HashMap.h>

#include "KI_Enums.h"

namespace KOS {
    class IWindow;
    class PCWindow;
}

namespace KI
{
    class InputSystem
    {
    public:
        static void Initialize();
        static void Shutdown();

        static void Reset();

        static bool IsKeyDown(Keycode code) { return Data.KeyStates[code].CurrentDown; }
        static bool IsKeyPressed(Keycode code) { return Data.KeyStates[code].CurrentDown && !Data.KeyStates[code].PrevDown; }
        static bool IsKeyReleased(Keycode code) { return !Data.KeyStates[code].CurrentDown && Data.KeyStates[code].PrevDown; }

        static glm::vec2 GetMousePosition() { return Data.MousePosition; }
        static glm::vec2 GetMouseDelta() { return Data.MouseDelta; }

        static bool IsMouseDown(MouseButton button) { return Data.MouseStates[button]; }
        static bool IsMouseUp(MouseButton button) { return !Data.MouseStates[button]; }
        static float GetMouseWheel() { return Data.MouseScroll; }    
    private:
        friend class KOS::IWindow;
        friend class KOS::PCWindow;

        struct KeyState {
            bool CurrentDown = false;
            bool PrevDown = false;
        };

        static struct InputData {
            glm::vec2 MousePosition;
            glm::vec2 MousePositionPrev;
            glm::vec2 MouseDelta;
            float MouseScroll;

            KC::HashMap<Keycode, KeyState> KeyStates;
            KC::HashMap<MouseButton, bool> MouseStates;
        } Data;
    };
}
