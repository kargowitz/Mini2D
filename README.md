# Mini2D
A tiny 2D Game and Engine in one (NOT a game engine)

# Mini 2D Engine

**Yes, I wrote my own malloc. Why? Because I can.**

**This is my first high-level C project**—a small 2D “game engine” that runs entirely in the terminal. It includes:

Custom memory allocator: A simple Malloc/Free implementation with block splitting and merging.

2D map rendering: Floor + player sprite, fully dynamic.

Raw terminal input: Move your player around with W, S, A, D, and trigger a “jump” with TAB.

No standard libraries beyond stdio.h and termios.

Build & Run

gcc render.c -o render
./render

Probs wont work on Windows
