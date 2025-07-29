//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 00:50:33
//

#include "KI_InputSystem.h"

#include <KernelCore/KC_Context.h>

namespace KI
{
    InputSystem::InputData InputSystem::Data;

    void InputSystem::Initialize()
    {
        for (int i = 0; i < (int)MouseButton::kMax; i++) {
            Data.MouseStates[(MouseButton)i] = false;
        }
        KD_INFO("Initialized input system!");
    }

    void InputSystem::Shutdown()
    {

    }

    void InputSystem::Reset()
    {
        // Get mouse position
        Data.MouseDelta = Data.MousePosition - Data.MousePositionPrev;
        Data.MousePositionPrev = Data.MousePosition;   

        // Update key states
        for (auto& [key, state] : Data.KeyStates) {
            state.PrevDown = state.CurrentDown;
        }

        // Reset
        KI::InputSystem::Data.MouseStates[KI::MouseButton::kMouseDown] = false;
        KI::InputSystem::Data.MouseStates[KI::MouseButton::kMouseUp] = false;
        KI::InputSystem::Data.MouseScroll = 0.0f;
    }
}
