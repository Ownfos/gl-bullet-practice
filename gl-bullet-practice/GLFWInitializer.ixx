module;

#include <GLFW/glfw3.h>
#include <stdexcept>

export module GLFWInitializer;

export namespace ownfos::opengl
{
    // RAII style initializer for automatic call to glfwInit() and glfwTerminate().
    // Create an instance at the beginning of your main function.
    class GLFWInitializer
    {
    public:
        GLFWInitializer()
        {
            if (glfwInit() == GLFW_FALSE) {
                throw std::runtime_error("[GLFWInitializer] Failed to initialize GLFW");
            }
        }

        ~GLFWInitializer()
        {
            glfwTerminate();
        }

    };
} // namespace ownfos::opengl
