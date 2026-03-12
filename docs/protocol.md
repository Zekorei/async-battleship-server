# Network Protocol

Client Commands:
-
| Command                     | Description                                           |
|-----------------------------|-------------------------------------------------------|
| `JOIN <name> <x> <y> <dir>` | Registers a player with a ship at the point (`x`,`y`) |
| `FIRE <x> <y>`              | Attacks the coordinate (`x`, `y`)                     |
| `PLAYERS`                   | Lists all connected players                           |
| `LEADERBOARD`               | Displays a leaderboard of players                     |
| `QUIT`                      | Disconnect from server                                |

---

Server Responses:
-
No response indicates success.

| Response                    | Description                                               |
|-----------------------------|-----------------------------------------------------------|
| `HIT <attacker> <defender>` | Player `<attacker>` hit player `<defender>` at (`x`, `y`) |
| `MISS <attacker>`           | Player `<attacker>` missed an attack                      |
| `SUNK <defender>`           | Player `<defender>`'s ship was sunk                       |
| `LEAVE <player>`            | Player `<player>` left the game                           |
