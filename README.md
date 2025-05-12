# Clone Robotics Coding Assignment

## Project Structure Overview

The application is split into two parts; a client and server. There is only one CMakeLists.txt file, which builds both binaries. Shared files (such as header definitions) are located in the `include/` folder.

Data transmission is unidirectional from the publisher to consumer. 

## Building the project

Building the project is relatively straight-forward.

```
mkdir build && cd build
cmake ../
make -j

# you may then run the pub/con as described in the PDF
publisher --socket-path /tmp/test_path --log-level INFO --frequency-hz 500
consumer --socket-path /tmp/test_path --log-level INFO --timeout-ms 100
```

Log levels include `INFO`, `ERROR`, or `NONE`.

## Design Choices

Much of the design was predetermined from the PDF (located in `docs/Coding_Task_Software_Roboticist_Engineer.pdf`). `AF_UNIX` was suggested through IPC and socket-path requirements, while `SOCK_SEQPACKET` seemed like a natural choice to avoid the complexity of streaming. Python also seemed like a poor choice given the optional RTOS requirement.

I made a decision early on that I would not git clone anything as well; such that all libraries had to be downloaded in the CMakeLists.txt file (i.e. `FetchContent_Declare`) or standard in C++. I decided to use `cxxopts.hpp` since it is both common and easy to use, however I decided to write my own logger instead of something like g3log, simply because I felt it would be easier in the context of this project.

My `static` global variables were also a bit of a tenuous design choice, as although I usually wouldn't write C++ programs this way I felt it was appropriate given the scope of the project.