export module PrimitiveModelData;

export import VertexBuffer;
export import IndexBuffer;

export namespace ownfos::primitives
{
    struct PrimitiveModelData
    {
        opengl::VertexBuffer<float> position;
        opengl::IndexBuffer indices;
    };
} // namespace ownfos::opengl
