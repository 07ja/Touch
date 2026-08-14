# Touch Application

#### A real-time physics application that uses deep learning based hand tracking to control a cursor and interact with objects

## How to Run
- Ensure the following dependencies are installed: **SDL3** and **ImGUI**
- Place the dependencies in the correct folders (see file structure below for reference)
- Run `make run` to build and launch the application
- Run `make clean` to remove the compiled file

## How to Use
- Add or remove circles to interact with the physics simulation
- Turn on the camera and enable real-time hand tracking
- Enable joint and bounding box visualizations to view hand detection results
- Use your index and middle fingers to point at and select circles
- Use a closed fist to grab/move and open your hand to release the circle

## File Structure
```
Application/
    ├── include/
    │   ├── imgui/ … 
    │   ├── controller.hpp
    │   ├── handPreprocess.hpp
    │   ├── handTrack.hpp
    │   ├── model.hpp
    │   ├── oneEuroFilter.hpp
    │   ├── onnxModel.hpp
    │   ├── renderImGUI.hpp
    │   ├── renderSDL.hpp
    │   └── view.hpp
    ├── src/
    │   ├── imgui/ …
    │   ├── controller.cpp
    │   ├── handPreprocess.cpp
    │   ├── handTrack.cpp
    │   ├── main.cpp
    │   ├── model.cpp
    │   ├── oneEuroFilter.cpp
    │   ├── onnxModel.cpp
    │   ├── renderImGUI.cpp
    │   ├── renderSDL.cpp
    │   └── view.cpp
    ├── Makefile
    └── README.md
```