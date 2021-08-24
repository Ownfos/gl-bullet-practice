module;

#include <vector>
#include <string>
#include <array>

export module ShaderSourceComposite;

export import IShaderSource;

export namespace ownfos::opengl
{
    // Provide concatenated version of multiple partial source codes.
    class ShaderSourceComposite : public IShaderSource
    {
    public:
        ShaderSourceComposite(const std::vector<IShaderSource*>& shader_source_container)
        {
            for (const IShaderSource* source : shader_source_container)
            {
                concatnated_shader_code.append(source->get_shader_code());
            }
        }

        std::string_view get_shader_code() const override
        {
            return concatnated_shader_code;
        }

    private:
        std::string concatnated_shader_code;
    };

    // Used to check shader preparation failure and provide buffer for error log messages
    template<size_t log_buffer_size = 512>
    struct ShaderInfoLog
    {
        int success;
        std::array<char, log_buffer_size> error_log;
    };
} // namespace ownfos::opengl
