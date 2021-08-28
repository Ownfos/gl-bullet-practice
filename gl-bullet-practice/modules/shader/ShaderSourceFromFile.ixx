module;

#include <fstream>
#include <fmt/format.h>

export module ShaderSourceFromFile;

export import IShaderSource;

export namespace ownfos::opengl
{
    // Provide content of text file as source code
    class ShaderSourceFromFile : public IShaderSource
    {
    public:
        ShaderSourceFromFile(std::string_view file_name)
        {
            // Open file
            auto file = std::ifstream(file_name.data());
            if (!file.is_open())
            {
                throw std::runtime_error(fmt::format("[ShaderSourceTextFile] Failed to open file \"{0}\"", file_name));
            }

            // Read content into std::string instance
            auto buffer = std::string(1024, '\0');
            while (file.read(&buffer[0], buffer.size()))
            {
                shader_code.append(buffer, 0, file.gcount());
            }
            shader_code.append(buffer, 0, file.gcount());
        }

        std::string_view get_shader_code() const override
        {
            return shader_code;
        }

    private:
        std::string shader_code;
    };
} // namespace ownfos::opengl
