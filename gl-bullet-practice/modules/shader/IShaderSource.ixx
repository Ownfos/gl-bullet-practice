module;

#include <string>

export module IShaderSource;

export namespace ownfos::opengl
{
    // Interface for classes that provide source code for vertex/fragment shader
    class IShaderSource
    {
    public:
        virtual std::string_view get_shader_code() const = 0;
    };
} // namespace ownfos::opengl
