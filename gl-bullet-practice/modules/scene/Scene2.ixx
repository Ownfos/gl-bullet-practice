module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/LinearMath/btVector3.h>
#include <iostream>
#include <exception>

export module Scene2;

import GLFWInitializer;
import Window;
import InteractiveCamera;
import CubeRenderer;
import LineRenderer;

using namespace ownfos::opengl;

export namespace scene2
{
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!", { 3, 3 });
            auto cube_renderer = CubeRenderer();
            auto line_renderer = LineRenderer();

            auto camera = InteractiveCamera(
                Camera{
                    .transform = {
                        .position = {0, 0, 10}
                    },
                    .projection = Projection::perspective(
                        glm::radians(45.0f),
                        window.get_aspect_ratio(),
                        0.1f,
                        1000.0f
                    )
                },
                window.get_normalized_cursor_pos()
            );
            window.set_cursor_enabled(false);

            window.register_key_handler([&](auto key, auto action) {
                if (key == GLFW_KEY_ESCAPE)
                {
                    window.set_close_flag();
                }
            });

            while (!window.should_close())
            {
                glfwPollEvents();

                camera.update_transform(window);

                window.clear({ 1,1,1,1 });

                auto cam_mat = camera.get_matrix();
                cube_renderer.render(cam_mat, glm::scale(glm::mat4(1.0f), { 0.5, 0.5, 0.5 }), { 0,1,1,1 });
                line_renderer.render(cam_mat, {0, 0, 0}, { 1,0,0 }, { 1,0,0,1 });
                line_renderer.render(cam_mat, {0, 0, 0}, { 0,1,0 }, { 0,1,0,1 });
                line_renderer.render(cam_mat, {0, 0, 0}, { 0,0,1 }, { 0,0,1,1 });

                window.swap_buffer();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} // namespace scene2
