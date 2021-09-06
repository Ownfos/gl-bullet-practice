# gl-bullet-practice
A test project for using OpenGL and Bullet Physics together, with latest support for c++20

## Project progression with GIFs
| ![hello world](https://user-images.githubusercontent.com/39623255/131685890-35762f2a-ce82-44e1-ae11-8a75e731bd74.gif) |
| :---: |
| *Hello world example* |

| ![camera movement](https://user-images.githubusercontent.com/39623255/132068145-f1ecb991-5be3-47c7-b9a2-fc5979f69055.gif) |
| :---: |
| *Camera movement with mouse and keyboard* |

| ![apply force](https://user-images.githubusercontent.com/39623255/132068167-4fa0fde3-b8ec-4905-b8be-bd37e687da9a.gif) |
| :---: |
| *Apply force along normal vector or camera ray direction* |

| ![drag object](https://user-images.githubusercontent.com/39623255/132099364-ba6664b5-50d9-48d4-96fc-26609fba3b2e.gif) |
| :---: |
| *Drag objects with mouse cursor* |

| ![drag object with gui](https://user-images.githubusercontent.com/39623255/132181253-3be96c66-9a73-47a9-b1f1-cc087645182b.gif) |
| :---: |
| *Add ImGui UI and damping* |

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
  - vcpkg install imgui[glfw-binding] --triplet=x64-windows
  - vcpkg install imgui[opengl3-glew-binding] --triplet=x64-windows
