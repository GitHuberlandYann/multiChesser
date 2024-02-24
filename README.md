# multiChesser
Multiplayer game of chess on local network.

## Introduction
While working on the project [VOX,](https://github.com/GitHuberlandYann/VOX.git) I figured it was time to implement multiplayer - on the local network.
This project serves as sandbox for me to get my hands on the communication protocol before I break my cherished VOX. And what other game to implement than the best ever created ? It's CHESS time boys.

## Building
```
$ git clone --recurse-submodules git@github.com:GitHuberlandYann/multiChesser.git
$ cd multiChesser
$ make setup
$ make
$ ./server

// open other terminal
$ ./client // (then press space)
// open other terminal
$ ./client // (then press space)
// open other terminal
$ ./client // (then press space)
// open other terminal
$ ./client // (then press space)
```
make setup will install the needed [static libraries.](#libraries)

make will create the needed executables - server and client.

The program ./server needs to be launched once, it will bind itself to the port 8080 and wait for connections.

The program ./client can be launched several times from different terminals, once you press space, they will connect to the server and the game begins.

## Libraries
* [GLFW](https://github.com/glfw/glfw.git) is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development. It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
* [GLEW](https://github.com/nigels-com/glew.git) provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform. I am using the [latest stable version.](https://github.com/nigels-com/glew/releases/tag/glew-2.2.0)
* [SOIL](https://github.com/littlstar/soil.git) is a tiny C library used primarily for uploading textures into OpenGL.

## Assets
Assets are coming from [chess.com](https://chess.com).
```
$ curl https://www.chess.com/chess-themes/pieces/neo/300/{w,b}{k,q,p,n,b,r}.png
```
