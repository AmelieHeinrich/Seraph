//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 18:14:55
//

#include "KOS_Window.PC.h"
#include "KOS_DirectAllocation.h"

#include <KernelInput/KI_InputSystem.h>

namespace KOS
{
    KI::Keycode SDLToKeyCode(SDL_Keycode code);
    KI::MouseButton SDLToMouseButton(SDL_MouseButtonFlags flags);

    IWindow* IWindow::Create(uint width, uint height, const char* title)
    {
        return KOS::New<PCWindow>(width, height, title);
    }

    PCWindow::PCWindow(uint width, uint height, const char* title)
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
        
#ifdef KD_MAC
        mWindow = SDL_CreateWindow(title, width, height, 0);
#else
        mWindow = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN);
#endif
        mRunning = true;
    }

    PCWindow::~PCWindow()
    {
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }

    bool PCWindow::IsOpen()
    {
        return mRunning;
    }

    bool PCWindow::PollEvents(void** event)
    {
        bool result = SDL_PollEvent(&mEvent);
        if (mEvent.type == SDL_EVENT_QUIT)
            mRunning = false;
        *event = (void*)&mEvent;
        UpdateInputSystem(mEvent);
        return result;
    }
    
    void PCWindow::UpdateInputSystem(SDL_Event& event)
    {
        // Mouse position
        SDL_GetGlobalMouseState(&KI::InputSystem::Data.MousePosition.x, &KI::InputSystem::Data.MousePosition.y);

        // Key events
        if (event.type == SDL_EVENT_KEY_DOWN) {
            SDL_Keycode code = event.key.key;
            KI::InputSystem::Data.KeyStates[SDLToKeyCode(code)].CurrentDown = true;
        }
        if (event.type == SDL_EVENT_KEY_UP) {
            SDL_Keycode code = event.key.key;
            KI::InputSystem::Data.KeyStates[SDLToKeyCode(code)].CurrentDown = false;
        }

        // Mouse events
        auto mouseFlags = SDL_GetGlobalMouseState(nullptr, nullptr);
        KI::InputSystem::Data.MouseStates[SDLToMouseButton(SDL_BUTTON_LEFT)] = (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT));
        KI::InputSystem::Data.MouseStates[SDLToMouseButton(SDL_BUTTON_RIGHT)] = (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT));
        KI::InputSystem::Data.MouseStates[SDLToMouseButton(SDL_BUTTON_MIDDLE)] = (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE));
        KI::InputSystem::Data.MouseStates[SDLToMouseButton(SDL_BUTTON_X1)] = (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_X1));
        KI::InputSystem::Data.MouseStates[SDLToMouseButton(SDL_BUTTON_X2)] = (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_X2));
        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            KI::InputSystem::Data.MouseScroll = event.wheel.y;
            if (event.wheel.y >= 0.001f) {
                KI::InputSystem::Data.MouseStates[KI::MouseButton::kMouseUp] = true;
            } else if (event.wheel.y <= -0.001f) {
                KI::InputSystem::Data.MouseStates[KI::MouseButton::kMouseDown] = true;
            }
        }
    }

    void* PCWindow::GetGPUSurface()
    {
#if defined(KD_WINDOWS)
        return SDL_GetPointerProperty(SDL_GetWindowProperties(mWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif
    }

    void PCWindow::GetSize(int& width, int& height)
    {
        SDL_GetWindowSize(mWindow, &width, &height);
    }

    void PCWindow::SetTitle(const char* title)
    {
        SDL_SetWindowTitle(mWindow, title);
    }

    const char* PCWindow::GetTitle()
    {
        return SDL_GetWindowTitle(mWindow);
    }

    KI::MouseButton SDLToMouseButton(SDL_MouseButtonFlags flags)
    {
        switch (flags) {
            case SDL_BUTTON_LEFT: return KI::MouseButton::kLeft;
            case SDL_BUTTON_MIDDLE: return KI::MouseButton::kMiddle;
            case SDL_BUTTON_RIGHT: return KI::MouseButton::kRight;
            case SDL_BUTTON_X1: return KI::MouseButton::kMouse4;
            case SDL_BUTTON_X2: return KI::MouseButton::kMouse5;
        }
        return KI::MouseButton::kLeft;
    }

    KI::Keycode SDLToKeyCode(SDL_Keycode code)
    {
        switch (code) {
            case SDLK_TAB: return KI::Keycode::kTab;
            case SDLK_LEFT: return KI::Keycode::kLeft;
            case SDLK_RIGHT: return KI::Keycode::kRight;
            case SDLK_UP: return KI::Keycode::kUp;
            case SDLK_DOWN: return KI::Keycode::kDown;
            case SDLK_PAGEUP: return KI::Keycode::kPageUp;
            case SDLK_PAGEDOWN: return KI::Keycode::kPageDown;
            case SDLK_HOME: return KI::Keycode::kHome;
            case SDLK_END: return KI::Keycode::kEnd;
            case SDLK_INSERT: return KI::Keycode::kInsert;
            case SDLK_DELETE: return KI::Keycode::kDelete;
            case SDLK_BACKSPACE: return KI::Keycode::kBackspace;
            case SDLK_SPACE: return KI::Keycode::kSpace;
            case SDLK_RETURN: return KI::Keycode::kReturn;
            case SDLK_ESCAPE: return KI::Keycode::kEscape;
            case SDLK_APOSTROPHE: return KI::Keycode::kApostrophe;
            case SDLK_COMMA: return KI::Keycode::kComma;
            case SDLK_MINUS: return KI::Keycode::kMinus;
            case SDLK_PERIOD: return KI::Keycode::kPeriod;
            case SDLK_SLASH: return KI::Keycode::kSlash;
            case SDLK_SEMICOLON: return KI::Keycode::kSemicolon;
            case SDLK_EQUALS: return KI::Keycode::kEquals;
            case SDLK_LEFTBRACKET: return KI::Keycode::kLeftBracket;
            case SDLK_BACKSLASH: return KI::Keycode::kBackslash;
            case SDLK_RIGHTBRACKET: return KI::Keycode::kRightBracket;
            case SDLK_GRAVE: return KI::Keycode::kGrave;
            case SDLK_CAPSLOCK: return KI::Keycode::kCapsLock;
            case SDLK_SCROLLLOCK: return KI::Keycode::kScrollLock;
            case SDLK_NUMLOCKCLEAR: return KI::Keycode::kNumLockClear;
            case SDLK_PRINTSCREEN: return KI::Keycode::kPrintScreen;
            case SDLK_PAUSE: return KI::Keycode::kPause;
            case SDLK_LCTRL: return KI::Keycode::kLCTRL;
            case SDLK_LSHIFT: return KI::Keycode::kLSHIFT;
            case SDLK_LALT: return KI::Keycode::kLALT;
            case SDLK_LGUI: return KI::Keycode::kLGUI;
            case SDLK_RCTRL: return KI::Keycode::kRCTRL;
            case SDLK_RSHIFT: return KI::Keycode::kRSHIFT;
            case SDLK_RALT: return KI::Keycode::kRALT;
            case SDLK_RGUI: return KI::Keycode::kRGUI;
            case SDLK_APPLICATION: return KI::Keycode::kApplication;
            case SDLK_0: return KI::Keycode::k0;
            case SDLK_1: return KI::Keycode::k1;
            case SDLK_2: return KI::Keycode::k2;
            case SDLK_3: return KI::Keycode::k3;
            case SDLK_4: return KI::Keycode::k4;
            case SDLK_5: return KI::Keycode::k5;
            case SDLK_6: return KI::Keycode::k6;
            case SDLK_7: return KI::Keycode::k7;
            case SDLK_8: return KI::Keycode::k8;
            case SDLK_9: return KI::Keycode::k9;
            case SDLK_A: return KI::Keycode::kA;
            case SDLK_B: return KI::Keycode::kB;
            case SDLK_C: return KI::Keycode::kC;
            case SDLK_D: return KI::Keycode::kD;
            case SDLK_E: return KI::Keycode::kE;
            case SDLK_F: return KI::Keycode::kF;
            case SDLK_G: return KI::Keycode::kG;
            case SDLK_H: return KI::Keycode::kH;
            case SDLK_I: return KI::Keycode::kI;
            case SDLK_J: return KI::Keycode::kJ;
            case SDLK_K: return KI::Keycode::kK;
            case SDLK_L: return KI::Keycode::kL;
            case SDLK_M: return KI::Keycode::kM;
            case SDLK_N: return KI::Keycode::kN;
            case SDLK_O: return KI::Keycode::kO;
            case SDLK_P: return KI::Keycode::kP;
            case SDLK_Q: return KI::Keycode::kQ;
            case SDLK_R: return KI::Keycode::kR;
            case SDLK_S: return KI::Keycode::kS;
            case SDLK_T: return KI::Keycode::kT;
            case SDLK_U: return KI::Keycode::kU;
            case SDLK_V: return KI::Keycode::kV;
            case SDLK_W: return KI::Keycode::kW;
            case SDLK_X: return KI::Keycode::kX;
            case SDLK_Y: return KI::Keycode::kY;
            case SDLK_Z: return KI::Keycode::kZ;
            case SDLK_F1: return KI::Keycode::kF1;
            case SDLK_F2: return KI::Keycode::kF2;
            case SDLK_F3: return KI::Keycode::kF3;
            case SDLK_F4: return KI::Keycode::kF4;
            case SDLK_F5: return KI::Keycode::kF5;
            case SDLK_F6: return KI::Keycode::kF6;
            case SDLK_F7: return KI::Keycode::kF7;
            case SDLK_F8: return KI::Keycode::kF8;
            case SDLK_F9: return KI::Keycode::kF9;
            case SDLK_F10: return KI::Keycode::kF10;
            case SDLK_F11: return KI::Keycode::kF11;
            case SDLK_F12: return KI::Keycode::kF12;
            case SDLK_F13: return KI::Keycode::kF13;
            case SDLK_F14: return KI::Keycode::kF14;
            case SDLK_F15: return KI::Keycode::kF15;
            case SDLK_F16: return KI::Keycode::kF16;
            case SDLK_F17: return KI::Keycode::kF17;
            case SDLK_F18: return KI::Keycode::kF18;
            case SDLK_F19: return KI::Keycode::kF19;
            case SDLK_F20: return KI::Keycode::kF20;
            case SDLK_F21: return KI::Keycode::kF21;
            case SDLK_F22: return KI::Keycode::kF22;
            case SDLK_F23: return KI::Keycode::kF23;
            case SDLK_F24: return KI::Keycode::kF24;
            case SDLK_AC_BACK: return KI::Keycode::kAC_Back;
            case SDLK_AC_FORWARD: return KI::Keycode::kAC_Forward;
        }
        return KI::Keycode::kTab;
    }
}
