module;

#include <gl/glew.h>
#include <stdexcept>
#include <format>

export module ShaderProgram;

export import ShaderSourceFromString;
export import ShaderSourceFromFile;
export import ShaderSourceComposite;

export namespace ownfos::opengl
{
    // Compiles and links vertex shader and fragment shader.
    // Call use() to activate this shader program.
    class ShaderProgram
    {
    public:
        ShaderProgram(const IShaderSource* vertex_shader_source, const IShaderSource* fragment_shader_source)
            : vertex_shader(glCreateShader(GL_VERTEX_SHADER))
            , fragment_shader(glCreateShader(GL_FRAGMENT_SHADER))
            , shader_program(glCreateProgram())
        {
            compile(vertex_shader, vertex_shader_source);
            compile(fragment_shader, fragment_shader_source);
            link();
        }

        void use() const
        {
            glUseProgram(shader_program);
        }

    private:
        void compile(unsigned int& shader, const IShaderSource* source)
        {
            const auto source_code = source->get_shader_code().data();
            glShaderSource(shader, 1, &source_code, nullptr);
            glCompileShader(shader);

            check_compile_error(shader, source);
        }

        void link()
        {
            glAttachShader(shader_program, vertex_shader);
            glAttachShader(shader_program, fragment_shader);
            glLinkProgram(shader_program);

            check_link_error();
        }

        void check_compile_error(unsigned int& shader, const IShaderSource* source)
        {
            auto info = ShaderInfoLog();

            glGetShaderiv(shader, GL_COMPILE_STATUS, &info.success);
            if (!info.success)
            {
                glGetShaderInfoLog(shader, info.error_log.size(), nullptr, info.error_log.data());
                throw std::runtime_error(std::format(
                    "[ShaderProgram] Failed to compile shader\n"
                    "--------------------------- Source code ---------------------------\n"
                    "{0}\n"
                    "-------------------------- Error Message --------------------------\n"
                    "{1}\n",
                    source->get_shader_code(),
                    info.error_log.data())
                );
            }
        }

        void check_link_error()
        {
            auto info = ShaderInfoLog();

            glGetProgramiv(shader_program, GL_LINK_STATUS, &info.success);
            if (!info.success) {
                glGetProgramInfoLog(shader_program, info.error_log.size(), nullptr, info.error_log.data());
                throw std::runtime_error(std::format(
                    "[ShaderProgram] Failed to link shaders into program\n"
                    "-------------------------- Error Message --------------------------\n"
                    "{0}",
                    info.error_log.data())
                );
            }
        }

        unsigned int vertex_shader;
        unsigned int fragment_shader;
        unsigned int shader_program;
    };
} // namespace ownfos::opengl
