# gl-bullet-practice
A test project for using OpenGL and Bullet Physics together, with latest support for c++20

## hello world example
![hello world](https://user-images.githubusercontent.com/39623255/131685890-35762f2a-ce82-44e1-ae11-8a75e731bd74.gif)

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
