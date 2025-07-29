//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:38:22
//

#include <catch2/catch_all.hpp>
#include <KernelCore/KC_Context.h>

int main (int argc, char * const argv[]) {
    KC::ScopedContext ctx;
    return Catch::Session().run( argc, argv );
}

