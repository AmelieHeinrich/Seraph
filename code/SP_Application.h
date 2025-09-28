//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:03:37
//

#pragma once

#include <KernelOS/KOS_Window.h>
#include <Graphics/Gfx_Manager.h>
#include <KernelCore/KC_Timer.h>

#include "SP_EditorCamera.h"
#include "Renderer/SP_RenderWorld.h"
#include "Renderer/SP_WorldRenderer.h"

namespace SP
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

        KOS::IWindow* GetWindow() { return mWindow; }
        static Application& Get() { return *sInstance; }
    private:
        void UI();

    private:
        static Application* sInstance;

        KC::Timer mClearTimer;
        KOS::IWindow* mWindow;
        uint mWidth = 1280;
        uint mHeight = 720;
        double mLast;

        KGPU::IDevice* mDevice;
        KGPU::ICommandQueue* mCommandQueue;
        KGPU::ISurface* mSurface;
        KGPU::ICommandList* mLists[KGPU::FRAMES_IN_FLIGHT];
        KGPU::ISync* mFrameSync;

        bool mUIOpened = false;
        bool mRendererSettingsOpened = false;
        bool mOverlayOpened = true;
        KC::String mStringBackend;

        EditorCamera mCamera;
        Gfx::Skybox mSky;
        RenderWorld* mWorld;
        WorldRenderer* mRenderer;
        RenderPassBegin mBegin;

        bool mPendingShaderReload = false;
        bool mPendingSkyboxReload = false;
        KC::String mSkyboxReloadPath = "";
        float mFontScale = 1.0f;
    };
}
