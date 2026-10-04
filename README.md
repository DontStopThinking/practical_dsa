# Practical Data Structures and Algorithms

I use this solution as a playground for learning various data structures and algorithms. Some of them contain
a README in their respective folders where I note down my explaination of them.

## Setup
This repository uses .slnx/.vcxproj project files created by Visual Studio 2026 Community Edition. Compiler
used is MSVC with C++23 Preview version.

I recomment the Smart Command Line Arguments Visual Studio extension to quickly run specific tests.

### Catch2
The examples for each data structure/algorithm are written as unit tests using the Catch2 library. Some
examples also contain some micro-benchmarks.

Launching the program in either Debug_Test or Release_Test configuration will run the unit tests.

Catch2 is installed using vcpkg.
