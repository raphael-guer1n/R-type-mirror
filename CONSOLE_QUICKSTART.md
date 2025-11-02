# Developer Console - Quick Start

## What Was Added

A server-side developer console has been added to your R-Type game for server administration and debugging. The console operates through the terminal where you start the server.

## Files Created

1. **src/server/ServerConsole.hpp** - Core console logic and command system
2. **src/server/ServerConsole.cpp** - Implementation of console functionality
3. **docs/developer_console.md** - Complete documentation

## Files Modified

- **src/server/Server.hpp** - Added console member and methods
- **src/server/Server.cpp** - Integrated console input/output and added game-specific commands
- **src/server/CMakeLists.txt** - Added console source files to build

## How to Use

The console runs **in the terminal where you start the server**, not in the game window.

### Start the Server:
```bash
./r-type_server
```

### Type Commands Directly:
```
help                          # Show all commands
players                       # List connected players
heal                         # Heal all players
spawn boss                   # Spawn a boss enemy
status                       # Show server status
stop                         # Stop server
```

## Key Features

**Server Management Commands**
- `help`, `status`, `stop`

**Player Management Commands** 
- `players`, `heal [player_index]`, `kill <player_index>`

**Entity Management Commands**
- `spawn <enemy_type>`, `clear`

**Quality of Life**
- Non-blocking input (doesn't interfere with game loop)
- Real-time command execution
- Error handling with helpful messages
- Works while server is running

## Architecture

The console system uses a clean separation of concerns:

- **ServerConsole** - Business logic (command registration, execution)
- **Server integration** - Game-specific commands and state access
- **Non-blocking I/O** - Uses `poll()` to check terminal input without blocking

## Available Commands

### Server Management
- `help` - Display all available commands
- `status` - Show server status (players, entities, tick count)
- `stop` - Gracefully stop the server

### Player Management
- `players` - List connected players with HP and IP
- `heal [player_index]` - Heal all players or specific player
- `kill <player_index>` - Kill a specific player

### Entity Management
- `spawn <enemy_type>` - Spawn enemies (crawler, shooter, boss)
- `clear` - Remove all enemies from the game

## Example Usage

```bash
# Start server
./r-type_server

# Once players connect, try these commands:
players                    # Check who's connected
heal                      # Heal everyone
spawn boss               # Add a boss for testing
status                   # Check server state
clear                    # Remove all enemies
stop                     # Shut down server
```

## Next Steps

To add your own commands, edit `setupConsoleCommands()` in Server.cpp:

```cpp
// Add a command
_console.registerCommand("mycommand", [this](const std::vector<std::string>& args) {
    _console.print("Hello from my command!");
}, "Does something cool");
```

See `docs/developer_console.md` for complete documentation.

## Technical Details

- **Location**: Server terminal (not client game window)
- **Input**: Non-blocking stdin reading with `poll()`
- **Output**: `[Console]` prefixed messages
- **Scope**: Affects entire server game state
- **Network**: Changes broadcast to all connected clients
- **Performance**: Zero impact on game loop timing
