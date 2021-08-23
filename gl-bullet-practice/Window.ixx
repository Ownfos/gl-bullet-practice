module;

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module Window;

import std.core;

export namespace ownfos::opengl
{
    struct GLFWContextVersion
    {
        int major;
        int minor;
    };

    // Creates a window and manages event handlers associated with that window
    class Window
    {
    public:
        using ResizeHandler = std::function<void(int, int)>;
        using KeyHandler = std::function<void(int, int)>;
        using MouseButtonHandler = std::function<void(int, int, int)>;
        using CursorPositionHandler = std::function<void(double, double)>;

        Window(int width, int height, const std::string& title, GLFWContextVersion context_version = { 3,3 })
            : width(width), height(height)
        {
            set_window_hint(context_version);

            // Create window
            window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
            if (window == NULL) {
                throw std::runtime_error("[Window] Failed to create a GLFW window");
            }

            set_event_callback();

            // Set the newly created window as the current context
            use_as_context();
        }

        ~Window()
        {
            if (window != NULL) {
                glfwDestroyWindow(window);
            }
        }

        int get_width() const
        {
            return width;
        }

        int get_height() const
        {
            return height;
        }

        float get_aspect_ratio() const
        {
            return static_cast<float>(width) / height;
        }

        void clear(const glm::vec4& rgba)
        {
            glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        // Make this window as active context.
        // Rendering will be done on this window's back buffer.
        void use_as_context()
        {
            glfwMakeContextCurrent(window);
            glEnable(GL_DEPTH_TEST);

            // Not sure why, but glewInit() seems mandatory when we use multiple windows.
            glewExperimental = GL_TRUE;
            if (glewInit() != GLEW_OK) {
                throw std::runtime_error("[Window] Failed to initialize GLEW");
            }
        }

        bool should_close() const
        {
            return glfwWindowShouldClose(window);
        }

        // Set close flag to true, so that should_close() returns true.
        void set_close_flag()
        {
            glfwSetWindowShouldClose(window, true);
        }

        void swap_buffer()
        {
            glfwSwapBuffers(window);
        }

        void register_resize_handler(ResizeHandler&& handler)
        {
            resize_handlers.push_back(std::move(handler));
        }

        void register_key_handler(KeyHandler&& handler)
        {
            key_handlers.push_back(std::move(handler));
        }

        void register_mouse_button_handler(MouseButtonHandler&& handler)
        {
            mouse_button_handlers.push_back(std::move(handler));
        }

        void register_cursor_position_handler(CursorPositionHandler&& handler)
        {
            cursor_position_handlers.push_back(std::move(handler));
        }

    private:
        void update_size_info(int new_width, int new_height)
        {
            width = new_width;
            height = new_height;

            glViewport(0, 0, width, height);
        }

        void set_window_hint(GLFWContextVersion context_version)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, context_version.major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, context_version.minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        }

        void set_event_callback()
        {
            // Register "this" to the GLFW window,
            // so that we can access this instance on glfw event callbacks.
            glfwSetWindowUserPointer(window, this);

            // Handle resize event
            glfwSetFramebufferSizeCallback(
                window,
                [](auto glfw_window, auto new_width, auto new_height) {
                    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

                    window->update_size_info(new_width, new_height);

                    for (auto& handler : window->resize_handlers) {
                        handler(new_width, new_height);
                    }
                }
            );

            // Handle keyboard input event
            glfwSetKeyCallback(
                window,
                [](auto glfw_window, auto key, auto, auto action, auto) {
                    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

                    for (auto& handler : window->key_handlers) {
                        handler(key, action);
                    }
                }
            );

            // Handle mouse input event
            glfwSetMouseButtonCallback(
                window,
                [](auto glfw_window, auto button, auto action, auto mods) {
                    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

                    for (auto& handler : window->mouse_button_handlers) {
                        handler(button, action, mods);
                    }
                }
            );

            // Handle mouse motion event
            glfwSetCursorPosCallback(
                window,
                [](auto glfw_window, auto x, auto y) {
                    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

                    for (auto& handler : window->cursor_position_handlers) {
                        handler(x, y);
                    }
                }
            );
        }

        int width;
        int height;

        GLFWwindow* window = nullptr;

        std::vector<ResizeHandler> resize_handlers;
        std::vector<KeyHandler> key_handlers;
        std::vector<MouseButtonHandler> mouse_button_handlers;
        std::vector<CursorPositionHandler> cursor_position_handlers;
    };
} // namespace ownfos::opengl
