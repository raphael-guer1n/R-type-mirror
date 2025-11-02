#include "Console.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

#ifndef _WIN32
#include <poll.h>
#include <unistd.h>
#endif

namespace engine {
namespace console {

Console::Console() {
    registerBuiltInCommands();
}

void Console::registerCommand(const std::string& name, CommandCallback callback, const std::string& help) {
    _commands[name] = callback;
    if (!help.empty()) {
        _commandHelp[name] = help;
    }
}

void Console::processInput(const std::string& input) {
    if (input.empty()) return;
    executeCommand(input);
}

void Console::print(const std::string& message) {
    std::cout << "[Console] " << message << std::endl;
}

void Console::executeCommand(const std::string& cmdLine) {
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

std::vector<std::string> Console::tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void Console::registerBuiltInCommands() {
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

// TerminalConsole implementation
TerminalConsole::TerminalConsole() : Console() {
    print("Type 'help' for available commands");
}

void TerminalConsole::checkInput() {
#ifndef _WIN32
    // Non-blocking input check for Unix systems
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    
    if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
        std::string line;
        if (std::getline(std::cin, line)) {
            if (!line.empty()) {
                processInput(line);
            }
        }
    }
#else
    // Windows implementation - simplified for now
    // TODO: Implement proper Windows non-blocking console input
    static bool warningShown = false;
    if (!warningShown) {
        print("Windows console input not implemented yet");
        warningShown = true;
    }
#endif
}

void TerminalConsole::print(const std::string& message) {
    std::cout << "[Engine Console] " << message << std::endl;
}

void TerminalConsole::registerBuiltInCommands() {
    Console::registerBuiltInCommands();
    
    registerCommand("clear", [this](const std::vector<std::string>&) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }, "Clear the console screen");
}

} // namespace console
} // namespace engine