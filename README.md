# gl-bullet-practice
A test project for using OpenGL and Bullet Physics together, with latest support for c++20

## hello world example
![hello world](https://user-images.githubusercontent.com/39623255/131685890-35762f2a-ce82-44e1-ae11-8a75e731bd74.gif)

## camera movement with mouse and keyboard
![camera movement](https://user-images.githubusercontent.com/39623255/132068145-f1ecb991-5be3-47c7-b9a2-fc5979f69055.gif)

## apply force along normal vector or camera ray direction
![apply force](https://user-images.githubusercontent.com/39623255/132068167-4fa0fde3-b8ec-4905-b8be-bd37e687da9a.gif)

## Project folder structure
- root
  - gl-bullet-practice
    - modules - OpenGL and Bullet Physics wrapper classes as module interface unit
    - resources - shaders written in GLSL
    - sources - main.cpp which contains testbench code

## Project setting
- Visual Studio 2022 Preview 3.1
- c++latest as standard
- x64 as build configuration
- vcpkg commands
  - vcpkg install glew --triplet=x64-windows
  - vcpkg install glfw3 --triplet=x64-windows
  - vcpkg install glm --triplet=x64-windows
  - vcpkg install fmt --triplet=x64-windows
  - vcpkg install bullet3 --triplet=x64-windows
