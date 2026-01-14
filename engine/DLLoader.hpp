/*
** EPITECH PROJECT, 2025
** archi_arcade
** File description:
** DLLoader
*/

#pragma once
#include <dlfcn.h>

#include <iostream>
#include <memory>
#include <string>

/**
 * @file DLLoader.hpp
 * @brief Header file for the DLLoader class template, which provides dynamic
 * library loading functionality.
 *
 * This class is part of the Arcade namespace and is designed to load shared
 * libraries dynamically, retrieve symbols, and create instances of objects
 * from the loaded libraries.
 */

namespace Arcade {
/**
 * @class DLLoaderException
 * @brief Exception class for dynamic library loading errors.
 *
 * This exception class is thrown when errors occur during library loading,
 * symbol retrieval, or instance creation in the DLLoader class.
 */
class DLLoaderException : public std::exception {
 public:
    /**
     * @brief Constructor for the DLLoaderException class.
     *
     * @param message The error message describing what went wrong.
     */
    explicit DLLoaderException(const std::string &message)
        : _message("[DLLoader] Error: " + message + ".") {}

    /**
     * @brief Override of what() method from std::exception.
     *
     * @return The error message as a C-string.
     */
    const char *what() const noexcept override {
        return _message.c_str();
    }

 private:
    std::string _message;  ///< The error message.
};

/**
 * @class DLLoader
 * @brief A template class for dynamically loading libraries and managing their
 * symbols.
 *
 * @tparam T The type of the object to be instantiated from the loaded library.
 */
template <typename T>
class DLLoader {
 public:
    /**
     * @brief Default constructor for the DLLoader class.
     *
     * Initializes the loader without loading any library.
     */
    DLLoader();

    /**
     * @brief Constructor that loads a library from the specified path.
     *
     * @param path The file path to the shared library to be loaded.
     */
    explicit DLLoader(const std::string &path);

    /**
     * @brief Destructor for the DLLoader class.
     *
     * Ensures that any loaded library is properly closed.
     */
    ~DLLoader();

    /**
     * @brief Opens a shared library from the specified path.
     *
     * @param path The file path to the shared library to be loaded.
     * @throws std::runtime_error If the library cannot be loaded.
     */
    void open(const std::string &path);

    /**
     * @brief Closes the currently loaded shared library.
     *
     * @throws std::runtime_error If no library is currently loaded.
     */
    void close();

    /**
     * @brief Retrieves a symbol from the loaded library.
     *
     * @tparam Func The type of the symbol (e.g., function pointer).
     * @param symbolName The name of the symbol to retrieve.
     * @return The symbol cast to the specified type.
     * @throws std::runtime_error If the symbol cannot be found or no library
     * is loaded.
     */
    template <typename Func>
    Func getSymbol(const std::string &symbolName);

    /**
     * @brief Creates an instance of the object using a factory function from
     * the library.
     *
     * @param creatorFuncName The name of the factory function (default is
     * "create").
     * @return A shared pointer to the created object.
     * @throws std::runtime_error If the factory function cannot be found or no
     * library is loaded.
     */
    std::shared_ptr<T> getInstance(
        const std::string &creatorFuncName = "create");

    /**
     * @brief Checks if a library is currently loaded.
     *
     * @return True if a library is loaded, false otherwise.
     */
    bool isLoaded() const;

    /**
     * @brief Gets the file path of the currently loaded library.
     *
     * @return The file path of the loaded library, or an empty string if no
     * library is loaded.
     */
    std::string getPath() const;

 private:
    void *_handle;      ///< Handle to the loaded library.
    std::string _path;  ///< File path of the loaded library.
};
}  // namespace Arcade

#include "DLLoader.tpp"
