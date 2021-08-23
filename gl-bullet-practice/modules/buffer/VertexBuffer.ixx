module;

#include <gl/glew.h>

export module VertexBuffer;

export import BufferDataVertex;
export import BufferDataIndex;

unsigned int create_gl_buffer()
{
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    return buffer;
}

unsigned int create_gl_vertex_array()
{
    unsigned int vertex_array;
    glGenVertexArrays(1, &vertex_array);
    return vertex_array;
}

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
        VertexBuffer(const std::vector<IBufferData*>& buffer_data_container, unsigned int usage)
            : vertex_array(create_gl_vertex_array())
        {
            glBindVertexArray(vertex_array);

            for (const auto data : buffer_data_container)
            {
                auto buffer = create_gl_buffer();

                data->bind_to(buffer, usage);

                buffers.push_back(buffer);
            }
        }

        void use() const
        {
            glBindVertexArray(vertex_array);
        }

    private:
        unsigned int vertex_array;
        std::vector<unsigned int> buffers;
    };
} // namespace ownfos::opengl
