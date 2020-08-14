# NDCAYF-Server
![cover art](/lore/EPACDestiny.jpg)
### "Numbers Don't Care About Your Feelers"
Lightweight first person shooter focused on multiplayer LAN-based gameplay, and geared towards native Linux compatibility.

## Contributors
* **Nathan Jonson** - [intrvirate](https://github.com/intrvirate)
* **Kevin Lockwood** - [kevin-b-lockwood](https://github.com/kevin-b-lockwood)
* **Matthew Tiemersma** - [NottMatt](https://github.com/NottMatt)
* **Luke Gantar** - [lukeg32](https://github.com/lukeg32)

## Objectives
* Lightweight FPS game
* Native Linux compatibility
* Lan capability

## Dependencies
* cmake
* assimp
* bullet

## Linux Installation:
1. Clone this git repo
2. Ensure you have all proper dependencies installed
3. `cd` into the repo directory and run `cmake ./*`
4. Run `make`
5. Run `./NDCAYF-Server`

## Windows Installation:
1. [Our more detailed, recommended method](https://en.wikipedia.org/wiki/Criticism_of_Linux)

## Current Progress
* Server response to pings
* can connect
* server can recieve from client and update accordingly
* server sends data 10/sec or 15/sec or 1/sec
* connection based off ip so clients can reconnect

## Todo
* add ability to see the lag in ms
* physics
* validation
* bullet stuff

## Links and Resources

### GitHub Repository
[NDCAYF](https://github.com/intrvirate/NDCAYF)
[NDCAYF-Server](https://github.com/Lukeg32/NDCAYF-Server)

### Lore and Story Concepts
[No_Longer_Human.pdf](https://github.com/intrvirate/NDCAYF/tree/master/lore)

### Acknowledgements
* Inspired in part by Halo CE 2001
* Written using OpenGL
* [This Article](https://www.gabrielgambetta.com/client-server-game-architecture.html)
