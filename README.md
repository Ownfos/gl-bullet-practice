# gl-bullet-practice
A test project for using OpenGL and Bullet Physics together, with latest support for c++20

## How to run specific scene
![image](https://user-images.githubusercontent.com/39623255/133563442-3c359d55-8dd2-4b33-b991-1fe4810e72b5.png)  
Call each scene's run() function defined in namespace sceneXXX.
```
sceneXXX::run();
```

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

| ![spring constraint](https://user-images.githubusercontent.com/39623255/132942354-95335315-4f45-4c99-8c63-26577168eedd.gif) |
| :---: |
| *Bullet's Spring constraint* |

| ![lagrangian mechanics](https://user-images.githubusercontent.com/39623255/133562041-e04faf13-cb60-4aca-9c11-77c1132baa29.gif) |
| :---: |
| *Single particle with radius constraint, using custom lagrangian mechanics simulator* |

| ![constraint control 1](https://user-images.githubusercontent.com/39623255/133600293-62ecbf32-33ef-4ed0-a9d0-53689ec61780.gif) |
| :---: |
| *Add real time gravity control* |

| ![constraint control 2](https://user-images.githubusercontent.com/39623255/133600415-44214c2b-ceee-4e66-8253-ff1bde57eed8.gif) |
| :---: |
| *Add constraint correction with real time parameter control* |
 

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
- Scene4.ixx
  - Test custom spring with spheres connected in cube/tetrahedron position
- Scene5.ixx
  - Test Bullet's built-in spring constraint with two cubes
- Scene6.ixx
  - Test lagrangian mechanics with single particle on plane with radius constraint
##### NOTE: Each test programs are separated into 'scene###.ixx' files under modules/scene folder

## Project setting
- Visual Studio 2022 Preview 4.0
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
