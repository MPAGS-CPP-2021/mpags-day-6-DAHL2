#include "CipherFactory.hpp"
#include "CipherMode.hpp"
#include "CipherType.hpp"
#include "CommandLineExceptions.hpp"
#include "ProcessCommandLine.hpp"
#include "TransformChar.hpp"

#include <cctype>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    // Convert the command-line arguments into a more easily usable form
    const std::vector<std::string> cmdLineArgs{argv, argv + argc};

    // Options that might be set by the command-line arguments
    ProgramSettings settings{
        false, false, "", "", "", CipherMode::Encrypt, CipherType::Caesar};

    // Process command line arguments
    try {
        processCommandLine(cmdLineArgs, settings);
    } catch (const MissingArgument& e) {
        std::cerr << "[error] Missing argument: " << e.what() << std::endl;
        return 2;
    } catch (const UnknownArgument& e) {
        std::cerr << "[error] Unknown argument: " << e.what() << std::endl;
        return 4;
    }

    // Handle help, if requested
    if (settings.helpRequested) {
        // Line splitting for readability
        std::cout
            << "Usage: mpags-cipher [-h/--help] [--version] [-i <file>] [-o <file>] [-c <cipher>] [-k <key>] [--encrypt/--decrypt]\n\n"
            << "Encrypts/Decrypts input alphanumeric text using classical ciphers\n\n"
            << "Available options:\n\n"
            << "  -h|--help        Print this help message and exit\n\n"
            << "  --version        Print version information\n\n"
            << "  -i FILE          Read text to be processed from FILE\n"
            << "                   Stdin will be used if not supplied\n\n"
            << "  -o FILE          Write processed text to FILE\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "  -c CIPHER        Specify the cipher to be used to perform the encryption/decryption\n"
            << "                   CIPHER can be caesar, playfair, or vigenere - caesar is the default\n\n"
            << "  -k KEY           Specify the cipher KEY\n"
            << "                   A null key is used if not supplied, which is only valid for the playfair cipher\n\n"
            << "  --encrypt        Will use the cipher to encrypt the input text (default behaviour)\n\n"
            << "  --decrypt        Will use the cipher to decrypt the input text\n\n"
            << std::endl;
        // Help requires no further action, so return from main
        // with 0 used to indicate success
        return 0;
    }

    // Handle version, if requested
    // Like help, requires no further action,
    // so return from main with zero to indicate success
    if (settings.versionRequested) {
        std::cout << "0.5.0" << std::endl;
        return 0;
    }

    // Initialise variables
    char inputChar{'x'};
    std::string inputText;

    // Read in user input from stdin/file
    if (!settings.inputFile.empty()) {
        // Open the file and check that we can read from it
        std::ifstream inputStream{settings.inputFile};
        if (!inputStream.good()) {
            std::cerr << "[error] failed to create istream on file '"
                      << settings.inputFile << "'" << std::endl;
            return 1;
        }

        // Loop over each character from the file
        while (inputStream >> inputChar) {
            inputText += transformChar(inputChar);
        }

    } else {
        // Loop over each character from user input
        // (until Return then CTRL-D (EOF) pressed)
        while (std::cin >> inputChar) {
            inputText += transformChar(inputChar);
        }
    }

    // Smart pointer would automatically point to nullptr anyway, but to be explicit...
    std::unique_ptr<Cipher> cipher{nullptr};

    // Request construction of the appropriate cipher
    try {
        cipher = cipherFactory(settings.cipherType, settings.cipherKey);
    } catch (const InvalidArgument& e) {
        std::cerr << "[error] InvalidArgument: " << e.what() << std::endl;
        return 3;
    }

    // Check that the cipher was constructed successfully
    if (!cipher) {
        std::cerr << "[error] problem constructing requested cipher"
                  << std::endl;
        return 1;
    }

    const size_t noThreads{4};
    const size_t inputSize{inputText.length()};
    size_t currentLocation{0};

    std::vector<std::string> splitInputText;

    for (size_t i{0}; i < noThreads; i++) {
        // This is bad: could screw up the playfair cipher by chopping up the string in the wrong place
        //  To implement that properly we would need to make the applyCipher function itself multithreaded,
        //  so at the moment, we just have to deal with extra characters potentially being added.
        //  (This might result in the program being unable to decrypt)
        splitInputText.push_back(inputText.substr(
            currentLocation,
            ((i + 1) * inputSize / noThreads) -
                currentLocation));    // Need to substract current location, since substr takes (start_pos, length_of_substr)
                                      // Not like python strings!!!
        currentLocation = (i + 1) * inputSize / noThreads;
    }

    std::vector<std::future<std::string>> futures;

    // Set the threads running
    // For some reason, get another segmentation error if I try to use the same loop as the one to get the input text
    for (size_t i{0}; i < noThreads; i++) {
        // It feels to me like this should be doable without a lambda, but I couldn't figure it out!
        auto threadCipher = [&cipher, &splitInputText, i, &settings]() {
            return cipher->applyCipher(splitInputText[i], settings.cipherMode);
        };

        futures.push_back(std::async(std::launch::async, threadCipher));
    }

    size_t currentCheckThread{0};
    std::future_status status{
        futures[currentCheckThread].wait_for(std::chrono::milliseconds(100))};
    // Wait for all threads to be finished
    do {
        if ((status == std::future_status::ready) &&
            (currentCheckThread < noThreads - 1)) {
            currentCheckThread++;
            status = futures[currentCheckThread].wait_for(
                std::chrono::milliseconds(1));
        } else {
            status = futures[currentCheckThread].wait_for(
                std::chrono::milliseconds(500));
        }
    } while (!((currentCheckThread == noThreads - 1) &&
               (status == std::future_status::ready)));

    // // Run the cipher on the input text, specifying whether to encrypt/decrypt
    // const std::string outputText{
    //     cipher->applyCipher(inputText, settings.cipherMode)};

    std::string outputText{""};
    for (size_t i{0}; i < noThreads; i++) {
        outputText += futures[i].get();
    }

    // Output the encrypted/decrypted text to stdout/file
    if (!settings.outputFile.empty()) {
        // Open the file and check that we can write to it
        std::ofstream outputStream{settings.outputFile};
        if (!outputStream.good()) {
            std::cerr << "[error] failed to create ostream on file '"
                      << settings.outputFile << "'" << std::endl;
            return 1;
        }

        // Print the encrypted/decrypted text to the file
        outputStream << outputText << std::endl;

    } else {
        // Print the encrypted/decrypted text to the screen
        std::cout << outputText << std::endl;
    }

    // No requirement to return from main, but we do so for clarity
    // and for consistency with other functions
    return 0;
}
