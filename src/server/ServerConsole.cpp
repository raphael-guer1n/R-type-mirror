#include "ServerConsole.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ServerConsole
{
    Console::Console()
    {
        registerBuiltInCommands();
        std::cout << "[Server Console] Type 'help' for available commands\n";
    }

    void Console::registerCommand(const std::string& name, CommandCallback callback, const std::string& help)
    {
        _commands[name] = callback;
        if (!help.empty()) {
            _commandHelp[name] = help;
        }
    }

    void Console::processInput(const std::string& input)
    {
        if (input.empty()) return;
        executeCommand(input);
    }

    void Console::print(const std::string& message)
    {
        std::cout << "[Console] " << message << std::endl;
    }

    void Console::executeCommand(const std::string& cmdLine)
    {
        auto tokens = tokenize(cmdLine);
        if (tokens.empty()) return;

        std::string cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        // Check if it's a command
        auto it = _commands.find(cmd);
        if (it != _commands.end()) {
            try {
                it->second(args);
            } catch (const std::exception& e) {
                print("Error: " + std::string(e.what()));
            }
            return;
        }

        print("Unknown command: " + cmd + ". Type 'help' for available commands.");
    }

    std::vector<std::string> Console::tokenize(const std::string& str)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }

    void Console::registerBuiltInCommands()
    {
        registerCommand("help", [this](const std::vector<std::string>&) {
            print("Available commands:");
            for (const auto& [name, callback] : _commands) {
                auto helpIt = _commandHelp.find(name);
                if (helpIt != _commandHelp.end()) {
                    print("  " + name + " - " + helpIt->second);
                } else {
                    print("  " + name);
                }
            }
        }, "Display this help message");
    }
}
