/*
** EPITECH PROJECT, 2025
** arcade
** File description:
** DLLoader
*/

#pragma once
#include <dlfcn.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <memory>
#include "DLLoader.hpp"

namespace Engine
{
    template <typename T>
    DLLoader<T>::DLLoader() : _handle(nullptr)
    {
    }

    template <typename T>
    DLLoader<T>::DLLoader(const std::string &path) : _handle(nullptr)
    {
        open(path);
    }

    template <typename T>
    DLLoader<T>::~DLLoader()
    {
        close();
    }

    template <typename T>
    void DLLoader<T>::open(const std::string &path)
    {
        close();
        _handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!_handle) {
            throw DLLoaderException(
                "Failed to load library: " + std::string(dlerror()));
        }
        _path = path;
    }

    template <typename T>
    void DLLoader<T>::close()
    {
        if (_handle) {
            dlclose(_handle);
            _handle = nullptr;
        }
    }

    template <typename T>
    template <typename Func>
    Func DLLoader<T>::getSymbol(const std::string &symbolName)
    {
        if (!_handle) {
            throw DLLoaderException("No library loaded");
        }
        dlerror();
        void *symbol = dlsym(_handle, symbolName.c_str());
        const char *error = dlerror();
        if (error) {
            throw DLLoaderException("Failed to get symbol '" + symbolName
                + "': " + std::string(error));
        }

        return reinterpret_cast<Func>(symbol);
    }

    template <typename T>
    std::shared_ptr<T> DLLoader<T>::getInstance(const std::string &creatorFuncName)
    {
        if (!_handle) {
            throw DLLoaderException("No library loaded");
        }

        dlerror();

        void *symbolPtr = dlsym(_handle, creatorFuncName.c_str());
        const char *error = dlerror();
        if (error) {
            throw DLLoaderException("Failed to get symbol '" + creatorFuncName + "': " + std::string(error));
        }

        if (!symbolPtr) {
            throw DLLoaderException("Symbol '" + creatorFuncName + "' is null");
        }

        using CreatorFunc = std::shared_ptr<T> (*)();
        CreatorFunc creator = reinterpret_cast<CreatorFunc>(symbolPtr);

        try {
            std::shared_ptr<T> instance;

            try {
                instance = creator();
                if (instance) {
                    return instance;
                }
            } catch (const std::exception &e) {
                std::cerr << "Standard approach failed: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Standard approach failed with unknown error" << std::endl;
            }

            throw DLLoaderException("Creator function failed to create a valid instance");
        } catch (const std::exception &e) {
            throw DLLoaderException("Error in creator function: " + std::string(e.what()));
        } catch (...) {
            throw DLLoaderException("Unknown error in creator function");
        }
    }

    template <typename T>
    bool DLLoader<T>::isLoaded() const
    {
        return _handle != nullptr;
    }

    template <typename T>
    std::string DLLoader<T>::getPath() const
    {
        return _path;
    }
}
