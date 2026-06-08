#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <memory>
#include <vector>

#include "scene/primitive.hpp"

namespace gc {

// Holds shared mesh data for all triangles in a mesh.
struct TriangleMesh {
    int n_triangles{ 0 };
    std::vector<Point3f> points;
    std::vector<Vector3f> normals;
    std::vector<Point2f> uvs;
    std::vector<int> vertex_indices;   // 3 per triangle
    std::vector<int> normal_indices;   // 3 per triangle (may be empty)
    std::vector<int> uv_indices;       // 3 per triangle (may be empty)
    bool backface_cull{ false };
};

// Creates the shared mesh and returns one Triangle per face.
std::vector<std::shared_ptr<Primitive>> create_triangle_mesh(
    std::shared_ptr<TriangleMesh> mesh,
    std::shared_ptr<Material> mat);

// Loads a TriangleMesh from a Wavefront OBJ file.
std::shared_ptr<TriangleMesh> load_mesh_from_obj(const std::string& filename);

class Triangle : public Primitive {
public:
    Triangle(std::shared_ptr<TriangleMesh> mesh, int face_idx,
             std::shared_ptr<Material> mat)
        : Primitive{ std::move(mat) }
        , mesh{ std::move(mesh) }
        , face_idx{ face_idx }
    {}

    bool intersect(const Ray& r, Surfel* sf) const override;
    bool intersect_p(const Ray& r) const override;
    [[nodiscard]] bool world_bounds(Bounds3f* bounds) const override;

private:
    std::shared_ptr<TriangleMesh> mesh;
    int face_idx;

    // Returns interpolated normal using barycentric coords (1-u-v, u, v).
    Vector3f interpolated_normal(real_type u, real_type v) const;
};

} // namespace gc

#endif // TRIANGLE_HPP
