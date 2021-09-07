module;

#include <gl/glew.h>
#include <glm/glm.hpp>

export module LineRenderer;

import ShaderProgram;
import BufferPreset;

export namespace ownfos::opengl
{
    class LineRenderer
    {
    public:
        LineRenderer()
            : vertex_shader_source("resources/line_vs.txt")
            , fragment_shader_source("resources/line_fs.txt")
            , shader(&vertex_shader_source, &fragment_shader_source)
            , position()
            , vertex_buffer(std::vector<float>(6, 0), { {.location = 0, .count = 3 } }, GL_DYNAMIC_DRAW)
            , buffer_preset({ &vertex_buffer })
        {}

        void render(const glm::mat4& camera, const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
        {
            shader.use();
            shader.set_uniform("camera", camera);
            shader.set_uniform("color", color);

            buffer_preset.use();

            // glm::vec3 has 3 tightly packed float, so we can use it as if it was float array.
            // const_cast was used because std::span only accepts non-const pointers (although we do not modify it)
            vertex_buffer.update_data(0, std::span{ const_cast<float*>(&start.x), 3 });
            vertex_buffer.update_data(3, std::span{ const_cast<float*>(&end.x), 3 });

            draw(GL_LINES, 2);
        }

    private:
        ShaderSourceFromFile vertex_shader_source;
        ShaderSourceFromFile fragment_shader_source;
        ShaderProgram shader;

        std::vector<float> position;
        VertexBuffer<float> vertex_buffer;
        BufferPreset buffer_preset;
    };
} // namespace ownfos::opengl
