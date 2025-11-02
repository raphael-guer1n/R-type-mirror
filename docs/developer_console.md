# Developer Console

## Overview

The R-Type server includes a terminal-based developer console for server administration, debugging, and game management. The console operates through the terminal where you start the server and provides essential commands for managing players, entities, and game state.

## How to Use

### Opening the Console
The console is **built into the server terminal** - there's no separate interface to open. Simply type commands directly in the terminal where you started the server.

### Starting the Server
```bash
./r-type_server
```

Once the server starts, you'll see:
```
UDP socket bound on port 4242
[Server Console] Type 'help' for available commands
Server Address: localhost (127.0.0.1)
Port: 4242
Waiting for 4 players...
```

You can now type commands directly in this terminal.

### Basic Usage
Simply type a command and press Enter:
```
help
status
players
```

## Available Commands

### Server Management
- `help` - Display all available commands with descriptions
- `status` - Show server status (players, entities, tick count, running state)
- `stop` - Gracefully stop the server

### Player Management  
- `players` - List all connected players with:
  - Player index [0], [1], etc.
  - Entity ID
  - Current HP
  - IP address and port
- `heal [player_index]` - Heal players
  - `heal` - Heal all players to full HP (20)
  - `heal 0` - Heal specific player by index
- `kill <player_index>` - Kill a specific player
  - Example: `kill 0` - Kill player 0

### Entity Management
- `spawn <enemy_type>` - Spawn enemies manually
  - `spawn crawler` - Spawn a basic crawler enemy
  - `spawn shooter` - Spawn a shooting enemy
  - `spawn boss` - Spawn a boss enemy
- `clear` - Remove all enemies from the game

## Command Examples

### Basic Server Administration
```bash
# Check server status
status

# List connected players
players

# Sample output:
# [Console] Connected Players:
# [Console]   [0] Entity: 0 HP: 16 @ 127.0.0.1:48763
# [Console]   [1] Entity: 1 HP: 14 @ 127.0.0.1:53681
```

### Player Management
```bash
# Heal all players
heal

# Heal specific player
heal 0

# Kill a player
kill 1
```

### Game Testing
```bash
# Spawn various enemies for testing
spawn crawler
spawn shooter  
spawn boss

# Clear all enemies
clear

# Check current game state
status
```

### Server Shutdown
```bash
# Stop the server gracefully
stop
```

## Technical Implementation

### Architecture
The console system consists of:
- **ServerConsole.hpp/cpp** - Core console logic and command registration
- **Server.cpp integration** - Game-specific commands and server state access
- **Non-blocking input** - Uses `poll()` to check for terminal input without blocking game loop

### Key Features
- **Real-time operation** - Commands execute immediately while server is running
- **Non-blocking** - Console input doesn't interfere with game loop performance
- **Error handling** - Invalid commands show helpful error messages
- **Help system** - Built-in command documentation
- **Player management** - Direct access to player health and state
- **Entity spawning** - Manual enemy creation for testing

### Command Registration
Server-specific commands are registered in `Server::setupConsoleCommands()`:

```cpp
_console.registerCommand("heal", [this](const std::vector<std::string>& args) {
    // Command implementation
}, "Command description");
```

## Use Cases

### Development & Testing
- **Enemy Testing**: Spawn specific enemy types to test behavior
- **Player Testing**: Heal/kill players to test game states
- **Performance Monitoring**: Use `status` to monitor entity counts
- **Game Flow Testing**: Clear enemies or spawn bosses manually

### Server Administration
- **Player Management**: Monitor player health and connection status
- **Game State Control**: Reset game state by clearing enemies
- **Server Monitoring**: Check tick rates and entity counts
- **Graceful Shutdown**: Stop server cleanly with `stop` command

### Debugging
- **Entity Tracking**: Monitor active entity counts with `status`
- **Player State**: Check individual player health with `players`
- **Manual Spawning**: Test specific game scenarios with `spawn`

## Differences from Client-Side Consoles

This is a **server-side administrative console**, not a client-side developer console like those found in games like Quake or Half-Life. Key differences:

- **Location**: Runs in server terminal, not game window
- **Purpose**: Server administration and debugging, not client-side cheats
- **Access**: Only server administrator has access
- **Scope**: Affects entire game state, not individual player experience
- **Networking**: Changes are immediately broadcast to all clients

## Console Output Format

All console output is prefixed with `[Console]` for easy identification:

```
[Console] Connected Players:
[Console]   [0] Entity: 0 HP: 20 @ 127.0.0.1:48763
[Console] Player 0 healed
[Console] Spawned boss enemy
[Console] Cleared 5 enemies
```

## Notes

- The console operates independently of client connections
- Commands execute immediately and effects are broadcast to all clients
- Player indices in commands correspond to connection order (first player = 0)
- Enemy types must match configuration files in `configs/enemy/`
- Server continues running normally while processing console commands
