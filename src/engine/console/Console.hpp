#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace engine {
namespace console {

/**
 * @class Console
 * @brief Engine developer console system
 */
class Console {
public:
    using CommandCallback = std::function<void(const std::vector<std::string>&)>;

    Console();
    virtual ~Console() = default;

    // Command registration
    void registerCommand(const std::string& name, CommandCallback callback, const std::string& help = "");

    // Input/Output
    virtual void processInput(const std::string& input);
    virtual void print(const std::string& message);

protected:
    void executeCommand(const std::string& cmdLine);
    std::vector<std::string> tokenize(const std::string& str);
    virtual void registerBuiltInCommands();

    std::unordered_map<std::string, CommandCallback> _commands;
    std::unordered_map<std::string, std::string> _commandHelp;
};

/**
 * @class TerminalConsole
 * @brief Terminal-based console implementation for server use
 */
class TerminalConsole : public Console {
public:
    TerminalConsole();
    void checkInput(); // Non-blocking input check
    void print(const std::string& message) override;

private:
    void registerBuiltInCommands() override;
};

} // namespace console
} // namespace engine