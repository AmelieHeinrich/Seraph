//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:38:39
//

#include "Gfx_CommandListRecycler.h"

namespace Gfx
{
    CommandListRecycler::Data CommandListRecycler::sData;

    void CommandListRecycler::Clean()
    {
        KD_ASSERT_EQ(!sData.mRecording, "Cannot clean recycler if it is in recording state!");

        KC_DELETE(sData.mActiveList);
    }

    KGPU::ICommandList* CommandListRecycler::RequestCommandList()
    {
        if (!sData.mRecording) {
            if (!sData.mActiveList) sData.mActiveList = Manager::GetCommandQueue()->CreateCommandList(false);
            sData.mActiveList->Reset();
            sData.mActiveList->Begin();
            sData.mRecording = true;
        }
        return sData.mActiveList;
    }

    void CommandListRecycler::FlushCommandLists()
    {
        if (!sData.mRecording) {
            KD_INFO("Nothing to flush :3");
            return;
        }

        sData.mActiveList->End();
        Manager::GetCommandQueue()->CommitCommandList(sData.mActiveList);
        sData.mRecording = false;
    }
}
