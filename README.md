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

| ![fps graph with vsync on](https://user-images.githubusercontent.com/39623255/132224110-febe4b56-e0e3-4f13-be4f-8d031f670f66.gif) |
| :---: |
| *FPS graph with VSync on* |

| ![fps graph with vsync off](https://user-images.githubusercontent.com/39623255/132224120-1284aba9-030c-4364-8295-2faf704b82b9.gif) |
| :---: |
| *FPS graph with VSync off* |

| ![spring parameters](https://user-images.githubusercontent.com/39623255/132481699-ca7af229-bd46-40a3-8c8f-d45677f94b00.gif) |
| :---: |
| *Two cubes connected with springs (feat. realtime spring coefficient & damping factor control)* |

| ![image](https://user-images.githubusercontent.com/39623255/132817594-78e2781c-6575-49ec-8835-aaaf4e418754.png) |
| :---: |
| *Renderers for smooth sphere (left), cube (top), flat sphere (right), and lines (xyz axis)* |

## Project folder structure
- root
  - gl-bullet-practice
    - modules - OpenGL and Bullet Physics wrapper classes as module interface unit
    - resources - shaders written in GLSL
    - sources - main.cpp which invokes testbench code implemented in 'Scene###.ixx' file

## Purpose of each scene
- Scene1.ixx
  - Create and render multiple cubes
  - Show FPS record as ImPlot bar graph
  - Drag cubes with mouse
- Scene2.ixx
  - Test primitive renderer functionality (line, cube, sphere)
- Scene3.ixx
  - Test multiple springs
  - Control spring strength and linear/angular damping with ImGui sliders
  - Drag cubes with mouse
##### NOTE: Each test programs are separated into 'scene###.ixx' files under modules/scene folder

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
  - vcpkg install implot --triplet=x64-windows
