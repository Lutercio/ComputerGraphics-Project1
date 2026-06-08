#include "scene/triangle.hpp"

#include <cmath>
#include <algorithm>

// For OBJ loading
#include "cyTriMesh.h"

namespace gc {

static constexpr real_type EPSILON = 1e-7f;

// Möller–Trumbore ray-triangle intersection.
// Returns true on hit and sets t, u, v (barycentric coords).
static bool moller_trumbore(const Ray& r,
                             const Point3f& p0, const Point3f& p1, const Point3f& p2,
                             real_type* t_out, real_type* u_out, real_type* v_out)
{
    Vector3f e1{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
    Vector3f e2{ p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };

    Vector3f h = cross(r.direction(), e2);
    real_type a = dot(e1, h);

    if (std::fabs(a) < EPSILON)
        return false; // ray parallel to triangle

    real_type f = 1.0f / a;
    Vector3f s{ r.origin().x - p0.x, r.origin().y - p0.y, r.origin().z - p0.z };

    real_type u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3f q = cross(s, e1);
    real_type v = f * dot(r.direction(), q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    real_type t = f * dot(e2, q);
    if (t < r.t_min_val() || t > r.t_max_val())
        return false;

    *t_out = t;
    *u_out = u;
    *v_out = v;
    return true;
}

bool Triangle::intersect_p(const Ray& r) const {
    int base = face_idx * 3;
    const Point3f& p0 = mesh->points[mesh->vertex_indices[base + 0]];
    const Point3f& p1 = mesh->points[mesh->vertex_indices[base + 1]];
    const Point3f& p2 = mesh->points[mesh->vertex_indices[base + 2]];

    // Backface culling: check if ray comes from the back
    if (mesh->backface_cull) {
        Vector3f e1{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
        Vector3f e2{ p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };
        Vector3f face_normal = cross(e1, e2);
        if (dot(r.direction(), face_normal) > 0.0f)
            return false;
    }

    real_type t, u, v;
    return moller_trumbore(r, p0, p1, p2, &t, &u, &v);
}

bool Triangle::intersect(const Ray& r, Surfel* sf) const {
    int base = face_idx * 3;
    const Point3f& p0 = mesh->points[mesh->vertex_indices[base + 0]];
    const Point3f& p1 = mesh->points[mesh->vertex_indices[base + 1]];
    const Point3f& p2 = mesh->points[mesh->vertex_indices[base + 2]];

    // Backface culling
    if (mesh->backface_cull) {
        Vector3f e1{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
        Vector3f e2{ p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };
        Vector3f face_normal = cross(e1, e2);
        if (dot(r.direction(), face_normal) > 0.0f)
            return false;
    }

    real_type t, u, v;
    if (!moller_trumbore(r, p0, p1, p2, &t, &u, &v))
        return false;

    if (sf != nullptr) {
        sf->p = r(t);
        sf->time = t;
        sf->wo = -r.direction();
        sf->primitive = this;

        // Normal: interpolated per-vertex if available, else geometric face normal
        if (!mesh->normals.empty() && !mesh->normal_indices.empty()) {
            sf->n = interpolated_normal(u, v);
        } else {
            Vector3f e1{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
            Vector3f e2{ p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };
            sf->n = normalize(cross(e1, e2));
        }

        // UV coordinates
        if (!mesh->uvs.empty() && !mesh->uv_indices.empty()) {
            const Point2f& uv0 = mesh->uvs[mesh->uv_indices[base + 0]];
            const Point2f& uv1 = mesh->uvs[mesh->uv_indices[base + 1]];
            const Point2f& uv2 = mesh->uvs[mesh->uv_indices[base + 2]];
            real_type w = 1.0f - u - v;
            sf->uv = Point2f{ w * uv0.x + u * uv1.x + v * uv2.x,
                              w * uv0.y + u * uv1.y + v * uv2.y };
        } else {
            sf->uv = Point2f{ u, v };
        }
    }

    return true;
}

Vector3f Triangle::interpolated_normal(real_type u, real_type v) const {
    int base = face_idx * 3;
    const Vector3f& n0 = mesh->normals[mesh->normal_indices[base + 0]];
    const Vector3f& n1 = mesh->normals[mesh->normal_indices[base + 1]];
    const Vector3f& n2 = mesh->normals[mesh->normal_indices[base + 2]];
    real_type w = 1.0f - u - v;
    return normalize(Vector3f{ w * n0.x + u * n1.x + v * n2.x,
                                w * n0.y + u * n1.y + v * n2.y,
                                w * n0.z + u * n1.z + v * n2.z });
}

bool Triangle::world_bounds(Bounds3f* bounds) const {
    if (!bounds) return false;
    int base = face_idx * 3;
    const Point3f& p0 = mesh->points[mesh->vertex_indices[base + 0]];
    const Point3f& p1 = mesh->points[mesh->vertex_indices[base + 1]];
    const Point3f& p2 = mesh->points[mesh->vertex_indices[base + 2]];
    *bounds = Bounds3f{
        Point3f{ std::min({p0.x, p1.x, p2.x}), std::min({p0.y, p1.y, p2.y}), std::min({p0.z, p1.z, p2.z}) },
        Point3f{ std::max({p0.x, p1.x, p2.x}), std::max({p0.y, p1.y, p2.y}), std::max({p0.z, p1.z, p2.z}) }
    };
    return true;
}

std::vector<std::shared_ptr<Primitive>> create_triangle_mesh(
    std::shared_ptr<TriangleMesh> mesh,
    std::shared_ptr<Material> mat)
{
    std::vector<std::shared_ptr<Primitive>> triangles;
    triangles.reserve(mesh->n_triangles);
    for (int i = 0; i < mesh->n_triangles; ++i)
        triangles.push_back(std::make_shared<Triangle>(mesh, i, mat));
    return triangles;
}

// Loads an OBJ file into a TriangleMesh using cyTriMesh.
std::shared_ptr<TriangleMesh> load_mesh_from_obj(const std::string& filename) {
    cy::TriMesh cy_mesh;
    if (!cy_mesh.LoadFromFileObj(filename.c_str(), false)) {
        return nullptr;
    }

    if (!cy_mesh.HasNormals()) {
        cy_mesh.ComputeNormals();
    }
    cy_mesh.ComputeBoundingBox();

    auto mesh = std::make_shared<TriangleMesh>();
    mesh->n_triangles = static_cast<int>(cy_mesh.NF());

    // Copy vertices
    mesh->points.reserve(cy_mesh.NV());
    for (unsigned int i = 0; i < cy_mesh.NV(); ++i) {
        const cy::Vec3f& v = cy_mesh.V(i);
        mesh->points.emplace_back(v.x, v.y, v.z);
    }

    // Copy normals
    mesh->normals.reserve(cy_mesh.NVN());
    for (unsigned int i = 0; i < cy_mesh.NVN(); ++i) {
        const cy::Vec3f& vn = cy_mesh.VN(i);
        mesh->normals.emplace_back(vn.x, vn.y, vn.z);
    }

    // Copy UV coords
    for (unsigned int i = 0; i < cy_mesh.NVT(); ++i) {
        const cy::Vec3f& vt = cy_mesh.VT(i);
        mesh->uvs.emplace_back(vt.x, vt.y);
    }

    // Copy face indices
    mesh->vertex_indices.reserve(cy_mesh.NF() * 3);
    for (unsigned int i = 0; i < cy_mesh.NF(); ++i) {
        const cy::TriMesh::TriFace& face = cy_mesh.F(i);
        mesh->vertex_indices.push_back(static_cast<int>(face.v[0]));
        mesh->vertex_indices.push_back(static_cast<int>(face.v[1]));
        mesh->vertex_indices.push_back(static_cast<int>(face.v[2]));
    }

    if (cy_mesh.HasNormals()) {
        mesh->normal_indices.reserve(cy_mesh.NF() * 3);
        for (unsigned int i = 0; i < cy_mesh.NF(); ++i) {
            const cy::TriMesh::TriFace& fn = cy_mesh.FN(i);
            mesh->normal_indices.push_back(static_cast<int>(fn.v[0]));
            mesh->normal_indices.push_back(static_cast<int>(fn.v[1]));
            mesh->normal_indices.push_back(static_cast<int>(fn.v[2]));
        }
    }

    if (cy_mesh.HasTextureVertices()) {
        mesh->uv_indices.reserve(cy_mesh.NF() * 3);
        for (unsigned int i = 0; i < cy_mesh.NF(); ++i) {
            const cy::TriMesh::TriFace& ft = cy_mesh.FT(i);
            mesh->uv_indices.push_back(static_cast<int>(ft.v[0]));
            mesh->uv_indices.push_back(static_cast<int>(ft.v[1]));
            mesh->uv_indices.push_back(static_cast<int>(ft.v[2]));
        }
    }

    return mesh;
}

} // namespace gc
