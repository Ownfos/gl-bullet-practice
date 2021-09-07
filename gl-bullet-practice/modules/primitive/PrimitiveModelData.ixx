export module PrimitiveModelData;

export import VertexBuffer;
export import IndexBuffer;

export namespace ownfos::primitive
{
    struct PrimitiveModelData
    {
        opengl::VertexBuffer<float> position;
        opengl::VertexBuffer<float> normal;
        opengl::IndexBuffer indices;
    };
} // namespace ownfos::opengl
