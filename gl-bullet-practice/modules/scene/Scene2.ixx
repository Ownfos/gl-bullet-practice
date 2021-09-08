module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/LinearMath/btVector3.h>
#include <iostream>
#include <exception>

export module Scene2;

import Window;
import InteractiveCamera;
import CubeRenderer;
import LineRenderer;

using namespace ownfos::opengl;

export namespace scene2
{
    // Test InteractiveCamera, CubeRenderer, and LineRenderer
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!", { 3, 3 });
            window.exit_when_pressed(GLFW_KEY_ESCAPE);

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
                window
            );

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
