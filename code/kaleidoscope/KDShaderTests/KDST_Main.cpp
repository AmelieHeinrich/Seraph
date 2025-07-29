//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:06:25
//

#include <catch2/catch_all.hpp>
#include <KernelCore/KC_Context.h>
#include <KDShader/KDS_Manager.h>

int main (int argc, char * const argv[]) {
    KC::ScopedContext ctx;
    KDS::Manager::Initialize();
    int code = Catch::Session().run( argc, argv );
    KDS::Manager::Shutdown();
    return code;
}
