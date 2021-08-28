module;

#include <gl/glew.h>
#include <vector>

export module VertexBuffer;

export import BufferDataVertex;
export import BufferDataIndex;

export namespace ownfos::opengl
{
    void draw(unsigned int primitive_mode, unsigned int num_vertices, unsigned int offset = 0)
    {
        glDrawArrays(primitive_mode, offset, num_vertices);
    }

    void draw_indexed(unsigned int primitive_mode, unsigned int num_vertices, unsigned int offset = 0)
    {
        glDrawElements(primitive_mode, num_vertices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
    }

    class VertexBuffer
    {
    public:
        VertexBuffer(const std::vector<IBufferData*>& buffer_data_container)
        {
            glGenVertexArrays(1, &vertex_array);
            glBindVertexArray(vertex_array);

            for (const auto data : buffer_data_container)
            {
                data->bind();
            }
        }

        void use() const
        {
            glBindVertexArray(vertex_array);
        }

    private:
        unsigned int vertex_array;
    };
} // namespace ownfos::opengl
