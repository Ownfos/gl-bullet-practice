module;

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdexcept>
#include <fmt/format.h>

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

        void set_uniform(std::string_view variable_name, bool value)
        {
            glUniform1f(uniform_location(variable_name), value);
        }
        
        void set_uniform(std::string_view variable_name, int value)
        {
            glUniform1i(uniform_location(variable_name), value);
        }

        void set_uniform(std::string_view variable_name, float value)
        {
            glUniform1f(uniform_location(variable_name), value);
        }

        void set_uniform(std::string_view variable_name, const glm::vec2& value)
        {
            glUniform2f(uniform_location(variable_name), value.x, value.y);
        }

        void set_uniform(std::string_view variable_name, const glm::vec3& value)
        {
            glUniform3f(uniform_location(variable_name), value.x, value.y, value.z);
        }

        void set_uniform(std::string_view variable_name, const glm::vec4& value)
        {
            glUniform4f(uniform_location(variable_name), value.x, value.y, value.z, value.w);
        }

        void set_uniform(std::string_view variable_name, const glm::mat4& value)
        {
            glUniformMatrix4fv(uniform_location(variable_name), 1, GL_FALSE, glm::value_ptr(value));
        }
        
    private:
        int uniform_location(std::string_view variable_name)
        {
            return glGetUniformLocation(shader_program, variable_name.data());
        }

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
                throw std::runtime_error(fmt::format(
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
                throw std::runtime_error(fmt::format(
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
