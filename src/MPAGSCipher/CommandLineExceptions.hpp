#ifndef MPAGSCIPHER_COMMANDLINEEXCEPTIONS_HPP
#define MPAGSCIPHER_COMMANDLINEEXCEPTIONS_HPP

#include <stdexcept>
#include <string>

/**
 * \file CommandLineExceptions.hpp
 * \brief Contains the exception classes for the possible errors in ProcessCommandLine
 */

/**
 * \class MissingArgument
 * \brief Exception to indicate the user has failed in pass a command line argument
 */
class MissingArgument : public std::invalid_argument {
  public:
    // Is this bad practice? Since everything is really only one line I thought it might be acceptable to keep everything in the header
    /**
     * \brief Create a new MissingArgument with the given message
     *
     * \param msg the message to be passed in the exception
     */
    explicit MissingArgument(const std::string& msg) :
        std::invalid_argument(msg){};
};

// These error are not actually thrown by the processCommandLine function.
//  but they arise form command line errors
/**
 * \class InvalidArgument
 * \brief Exception to indicate the user has passed an argument with the wrong format
 */
class InvalidArgument : public std::invalid_argument {
  public:
    /**
     * \brief Create a new InvalidArgument with the given message
     *
     * \param msg the message to be passed in the exception
     */
    explicit InvalidArgument(const std::string& msg) :
        std::invalid_argument(msg){};
};

/**
 * \class UnknownArgument
 * \brief Exception to indicate the user has passed an unexpected argument
 */
class UnknownArgument : public std::invalid_argument {
  public:
    /**
     * \brief Create a new UnknownArgument with the given message
     *
     * \param msg the message to be passed in the exception
     */
    explicit UnknownArgument(const std::string& msg) :
        std::invalid_argument(msg){};
};

#endif    // MPAGSCIPHER_COMMANDLINEEXCEPTIONS_HPP
