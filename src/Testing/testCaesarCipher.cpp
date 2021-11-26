//! Unit Tests for MPAGSCipher CaesarCipher Class
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "CaesarCipher.hpp"
#include "CommandLineExceptions.hpp"

TEST_CASE("Caesar Cipher encryption", "[caesar]")
{
    CaesarCipher cc{10};
    REQUIRE(cc.applyCipher("HELLOWORLD", CipherMode::Encrypt) == "ROVVYGYBVN");
}

TEST_CASE("Caesar Cipher decryption", "[caesar]")
{
    CaesarCipher cc{10};
    REQUIRE(cc.applyCipher("ROVVYGYBVN", CipherMode::Decrypt) == "HELLOWORLD");
}

TEST_CASE("Caesar Cipher empty key", "[caesar]")
{
    REQUIRE_THROWS_AS(CaesarCipher(""), InvalidArgument);
}

TEST_CASE("Caesar Cipher no number key", "[caesar]")
{
    REQUIRE_THROWS_AS(CaesarCipher("fdshabgd"), InvalidArgument);
}

TEST_CASE("Caesar Cipher negative key", "[caesar]")
{
    REQUIRE_THROWS_AS(CaesarCipher("-12"), InvalidArgument);
}
