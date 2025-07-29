//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:53:09
//

#include <KernelCore/KC_StringStream.h>
#include <catch2/catch_test_macros.hpp>

using namespace KC;

TEST_CASE("StringStream write and size", "[StringStream]") {
    StringStream stream;

    const char* data = "hello";
    stream.Write(data, 5);
    REQUIRE(stream.Size() == 5);
    REQUIRE(stream.Tell() == 5);

    KC::String str(" world");
    stream.Write(str);
    REQUIRE(stream.Size() == 11);
    REQUIRE(stream.Tell() == 11);
}

TEST_CASE("StringStream read", "[StringStream]") {
    StringStream stream;
    stream.Write("abcdef", 6);

    char buffer[4] = {};
    stream.Seek(0);
    uint64 readSize = stream.Read(buffer, 3);
    REQUIRE(readSize == 3);
    REQUIRE(buffer[0] == 'a');
    REQUIRE(buffer[1] == 'b');
    REQUIRE(buffer[2] == 'c');

    readSize = stream.Read(buffer, 3);
    REQUIRE(readSize == 3);
    REQUIRE(buffer[0] == 'd');
    REQUIRE(buffer[1] == 'e');
    REQUIRE(buffer[2] == 'f');
}

TEST_CASE("StringStream seek and tell", "[StringStream]") {
    StringStream stream;
    stream.Write("1234567890", 10);

    REQUIRE(stream.Seek(5) == true);
    REQUIRE(stream.Tell() == 5);

    char c = 0;
    stream.Read(&c, 1);
    REQUIRE(c == '6');

    REQUIRE(stream.Seek(100) == false); // beyond size
    REQUIRE(stream.Tell() == 6);        // should not have moved
}

TEST_CASE("StringStream read beyond end", "[StringStream]") {
    StringStream stream;
    stream.Write("hi", 2);

    stream.Seek(0);
    char buffer[4] = {};
    uint64 readSize = stream.Read(buffer, 4);
    REQUIRE(readSize == 2);
    REQUIRE(buffer[0] == 'h');
    REQUIRE(buffer[1] == 'i');
}

TEST_CASE("StringStream append behavior", "[StringStream]") {
    StringStream stream;
    stream.Write("abcdef", 6);
    stream.Seek(6);
    stream.Write("XYZ", 3);

    REQUIRE(stream.Size() == 9);  // abcdefXYZ

    stream.Seek(0);
    char buffer[10] = {};  // Zero initialize the full buffer
    uint64 bytesRead = stream.Read(buffer, 9);
    REQUIRE(bytesRead == 9);
    REQUIRE(KC::String(buffer, bytesRead) == "abcdefXYZ");  // Explicitly specify string length
}

TEST_CASE("StringStream partial write and read", "[StringStream]") {
    StringStream stream;
    stream.Write("test", 4);
    stream.Seek(2);
    stream.Write("mp", 2);

    char buffer[5] = {};
    stream.Seek(0);
    uint64 bytesRead = stream.Read(buffer, 4);
    REQUIRE(bytesRead == 4);
    REQUIRE(KC::String(buffer, bytesRead) == "temp");
}
