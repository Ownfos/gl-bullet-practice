#include <gl/glew.h>
#include <GLFW/glfw3.h>

import GLFWInitializer;
import Window;
import ShaderProgram;
import VertexBuffer;

import Plane;
import Cube;

using namespace ownfos::opengl;
using namespace ownfos::primitives;

int main()
{
    try
    {
        auto init = GLFWInitializer();
        auto window = Window(600, 400, "hello, world!");
        window.register_key_handler([&](auto key, auto action)
            {
                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                {
                    window.set_close_flag();
                }
            });

        auto vs = ShaderSourceFromFile("resources/default_vs.txt");
        auto fs = ShaderSourceFromFile("resources/default_fs.txt");
        auto shader = ShaderProgram(&vs, &fs);

        auto color = std::vector<float>{
            1,0,1,1,
            1,0,0,1,
            0,1,0,1,
            0,0,1,1
        };
        auto color_data = BufferDataVertex(std::move(color), {{.location = 1, .count = 4}});

        auto data = plane::create_data();

        auto model = VertexBuffer({ &data.position, &data.indices, &color_data }, GL_STATIC_DRAW);

        while (!window.should_close())
        {
            window.clear({ 0,1,1,1 });

            shader.use();
            model.use();
            draw_indexed(GL_TRIANGLES, 6, 0);

            window.swap_buffer();

            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
