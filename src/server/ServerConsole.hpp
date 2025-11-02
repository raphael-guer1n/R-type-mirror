#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace ServerConsole
{
    /**
     * @class Console
     * @brief Server developer console for executing commands via stdin
     */
    class Console
    {
    public:
        using CommandCallback = std::function<void(const std::vector<std::string>&)>;

        Console();
        ~Console() = default;

        // Command registration
        void registerCommand(const std::string& name, CommandCallback callback, const std::string& help = "");

        // Input handling
        void processInput(const std::string& input);
        void print(const std::string& message);

    private:
        void executeCommand(const std::string& cmdLine);
        std::vector<std::string> tokenize(const std::string& str);
        void registerBuiltInCommands();

        std::unordered_map<std::string, CommandCallback> _commands;
        std::unordered_map<std::string, std::string> _commandHelp;
    };
}
