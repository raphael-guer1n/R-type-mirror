#pragma once
#include <exception>
#include <string>

namespace R_Graphic {
class Error : public std::exception
{
    public:
        explicit Error(const std::string& message = "ERROR") : _message(message) {}
        ~Error() override = default;
        const char *what() const noexcept override {
            return _message.c_str();
        }
    protected:
    private:
        std::string _message;
};
}