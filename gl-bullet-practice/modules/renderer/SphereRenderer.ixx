module;

#include <gl/glew.h>
#include <glm/glm.hpp>

export module SphereRenderer;

import ShaderProgram;
import BufferPreset;
import Sphere;

using namespace ownfos::primitive;

export namespace ownfos::opengl
{
    class SphereRenderer
    {
    public:
        SphereRenderer(int num_vertices_per_half_circle = 20, bool is_surface_smooth = true)
            : vertex_shader_source("resources/simple_mesh_vs.txt")
            , fragment_shader_source("resources/simple_mesh_fs.txt")
            , shader(&vertex_shader_source, &fragment_shader_source)
            , sphere_data(create_sphere_model_data(num_vertices_per_half_circle, is_surface_smooth))
            , buffer_preset({ &sphere_data.position, &sphere_data.normal, &sphere_data.indices })
        {}

        void render(const glm::mat4& camera, const glm::mat4& transform, const glm::vec4& color)
        {
            shader.use();
            shader.set_uniform("camera", camera);
            shader.set_uniform("world", transform);
            shader.set_uniform("color", color);

            buffer_preset.use();
            draw_indexed(GL_TRIANGLES, sphere_data.indices.get_num_indices(), 0);
        }

    private:
        ShaderSourceFromFile vertex_shader_source;
        ShaderSourceFromFile fragment_shader_source;
        ShaderProgram shader;

        PrimitiveModelData sphere_data;
        BufferPreset buffer_preset;
    };
} // namespace ownfos::opengl
