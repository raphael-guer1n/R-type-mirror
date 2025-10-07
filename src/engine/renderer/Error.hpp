#pragma once
#include <exception>
#include <string>

/**
 * @file Error.hpp
 * @brief Defines the Error exception class for the R_Graphic renderer engine.
 *
 * This header declares a custom exception class used to signal errors
 * within the renderer module of the R-type project. The Error class
 * inherits from std::exception and allows for descriptive error messages.
 */
namespace engine {
    namespace R_Graphic
    {
        class Error : public std::exception
        {
            public:
                explicit Error(const std::string &message = "ERROR") : _message(message) {}
                ~Error() override = default;
                const char *what() const noexcept override
                {
                    return _message.c_str();
                }

            protected:
            private:
                std::string _message;
        };
    }
}