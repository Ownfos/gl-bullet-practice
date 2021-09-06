module;

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <string>
#include <stdexcept>

export module Window;

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
        // Parameters:
        // - width: horizontal size of window in pixels
        // - height: vertical size of window in pixels
        using ResizeHandler = std::function<void(int /* width */, int /* height */)>;

        // Parameters:
        // - key: GLFW_KEY_XXX corresponding to the pressed/released key (e.g. GLFW_KEY_F if key 'f' was pressed)
        // - action: GLFW_PRESS or GLFW_RELEASE
        using KeyHandler = std::function<void(int /* key */, int /* action */)>;

        // Parameters:
        // - button: GLFW_MOUSE_BUTTON_XXX corresponding to the pressed/released mouse button (e.g. GLFW_MOUSE_BUTTON_LEFT if mouse left button was clicked)
        // - action: GLFW_PRESS or GLFW_RELEASE
        // - mods: bitmask corresponding to zero or more GLFW_MOD_XXX flags 'OR'ed together (e.g. GLFW_MOD_SHIFT if any shift key was pressed at the moment)
        using MouseButtonHandler = std::function<void(int /* button */, int /* action */, int /* mods */)>;

        // Parameters:
        // - x: distance in pixels from the left edge of this window
        // - y: distance in pixels from the top edge of this window
        using CursorPositionHandler = std::function<void(double /* x */, double /* y */)>;

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

        GLFWwindow* get_glfw_window()
        {
            return window;
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
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        // Returns the lastly updated key state (GLFW_PRESS or GLFW_RELEASE)
        int get_key_state(int key) const
        {
            return glfwGetKey(window, key);
        }

        // Returns the lastly updated mouse button state (GLFW_PRESS or GLFW_RELEASE)
        int get_mouse_button_state(int button) const
        {
            return glfwGetMouseButton(window, button);
        }

        // Returns the cursor's coordinate in normalized space (-1 to +1),
        // where left-bottom corner corresponds to (-1, -1)
        glm::vec2 get_normalized_cursor_pos() const
        {
            // Get the cursor coordinate in pixels, where left-top corner is (0, 0) and right-bottom is (width, height) in pixels.
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            // Normalize the coordinate, so that it ranges from -1 to +1.
            // 
            // Note that y-axis of raw cursor pos points down.
            // This means that the y value becomes equal to the
            // window height (in pixels) when the cursor reaches the bottom.
            // 
            // We need to invert the direction by adding minus sign for y coordinate.
            x = x / width * 2.0f - 1.0f;
            y = -(y / height * 2.0f - 1.0f);

            return { x, y };
        }

        // Enable or disable mouse cursor.
        // When cursor is disabled, it becomes invisible and locked inside the window.
        void set_cursor_enabled(bool enabled)
        {
            glfwSetInputMode(window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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
