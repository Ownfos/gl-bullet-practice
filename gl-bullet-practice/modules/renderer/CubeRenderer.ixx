module;

#include <gl/glew.h>
#include <glm/glm.hpp>

export module CubeRenderer;

import ShaderProgram;
import BufferPreset;
import Cube;

using namespace ownfos::primitive;

export namespace ownfos::opengl
{
    class CubeRenderer
    {
    public:
        CubeRenderer()
            : vertex_shader_source("resources/cube_vs.txt")
            , fragment_shader_source("resources/cube_fs.txt")
            , shader(&vertex_shader_source, &fragment_shader_source)
            , cube_data(create_cube_model_data())
            , buffer_preset({ &cube_data.position, &cube_data.normal, &cube_data.indices })
        {}

        void render(const glm::mat4& camera, const glm::mat4& transform, const glm::vec4& color)
        {
            shader.use();
            shader.set_uniform("camera", camera);
            shader.set_uniform("world", transform);
            shader.set_uniform("color", color);

            buffer_preset.use();
            draw_indexed(GL_TRIANGLES, 36, 0);
        }

    private:
        ShaderSourceFromFile vertex_shader_source;
        ShaderSourceFromFile fragment_shader_source;
        ShaderProgram shader;

        PrimitiveModelData cube_data;
        BufferPreset buffer_preset;
    };
} // namespace ownfos::opengl
