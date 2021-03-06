#include "CaesarCipher.hpp"
#include "Alphabet.hpp"
#include "CommandLineExceptions.hpp"

#include <iostream>
#include <string>

CaesarCipher::CaesarCipher(const std::size_t key) : key_{key % Alphabet::size}
{
}

CaesarCipher::CaesarCipher(const std::string& key) : key_{0}
{
    // We have the key as a string, but the Caesar cipher needs an unsigned long, so we first need to convert it
    // We default to having a key of 0, i.e. no encryption, if no (valid) key was provided on the command line
    if (!key.empty()) {
        // Explicitly check the user hasn't tried to input a negative number
        if (key.front() == '-') {
            // Not sure how to format this string nicely!
            throw InvalidArgument(
                "cipher key must be an unsigned long integer for Caesar cipher,\n        the supplied key (" +
                key + ") appears to be a negative number");
        }
        try {
            key_ = std::stoul(key) % Alphabet::size;
        } catch (std::invalid_argument& e) {
            // N.B. this error is only thrown if ALL characters are not integers: it will still pass
            //  if i.e. we type "1banana": this isn't ideal so the previous method of handling this might be better!
            throw InvalidArgument(
                "cipher key must be an unsigned long integer for Caesar cipher,\n        the supplied key (" +
                key + ") does not contain any numbers");
        } catch (std::out_of_range& e) {
            throw InvalidArgument(
                "the supplied cipher key is too large to be converted to an unsigned long");
        }
    } else {
        throw InvalidArgument("a cipher key must be supplied");
    }
}

std::string CaesarCipher::applyCipher(const std::string& inputText,
                                      const CipherMode cipherMode) const
{
    // Create the output string
    std::string outputText;

    // Loop over the input text
    char processedChar{'x'};
    for (const auto& origChar : inputText) {
        // For each character in the input text, find the corresponding position in
        // the alphabet by using an indexed loop over the alphabet container
        for (std::size_t i{0}; i < Alphabet::size; ++i) {
            if (origChar == Alphabet::alphabet[i]) {
                // Apply the appropriate shift (depending on whether we're encrypting
                // or decrypting) and determine the new character
                // Can then break out of the loop over the alphabet
                switch (cipherMode) {
                    case CipherMode::Encrypt:
                        processedChar =
                            Alphabet::alphabet[(i + key_) % Alphabet::size];
                        break;
                    case CipherMode::Decrypt:
                        processedChar =
                            Alphabet::alphabet[(i + Alphabet::size - key_) %
                                               Alphabet::size];
                        break;
                }
                break;
            }
        }

        // Add the new character to the output text
        outputText += processedChar;
    }

    return outputText;
}