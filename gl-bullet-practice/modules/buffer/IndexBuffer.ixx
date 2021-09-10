module;

#include <gl/glew.h>
#include <vector>

export module IndexBuffer;

export import IBuffer;

export namespace ownfos::opengl
{
    class IndexBuffer : public IBuffer
    {
    public:
        IndexBuffer(const std::vector<unsigned int>& indices, unsigned int usage = GL_STATIC_DRAW)
            : num_indices(indices.size())
        {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], usage);
        }

        void bind() const override
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        }

        int get_num_indices() const
        {
            return num_indices;
        }

    private:
        unsigned int buffer;
        int num_indices;
    };
} // namespace ownfos::opengl
