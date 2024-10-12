#include "geometry_func.hpp"
#include <array>
#include "debug/log.hpp"
#include "math/math_func.hpp"
namespace emp {
bool isTriangulable(const std::vector<vec2f>& points) {
    for (int i = 0; i < points.size(); i++) {
        auto first = points[i];
        auto mid_index = (i + 1) % points.size();
        auto mid = points[mid_index];
        auto last = points[(i + 2) % points.size()];
        float angle = angleAround(first, mid, last);
        if (angle < 0.f) {
            continue;
        }
        bool containsVertex = false;
        for (auto p : points) {
            if (p == first || p == mid || p == last)
                continue;
            if (isOverlappingPointPoly(p, {first, mid, last})) {
                containsVertex = true;
                break;
            }
        }
        if (containsVertex) {
            continue;
        }
        return true;
    }
    return false;
}

template <class ResultT>
std::vector<ResultT> triangulateEarClipping(const std::vector<vec2f>& points) {
    std::vector<ResultT> result;
    std::vector<vec2f> tmp = points;
    while (result.size() != points.size() - 2) {
        bool expandedResult = false;
        int begin = rand() % tmp.size();
        for (int ii = begin; ii < begin + tmp.size(); ii++) {
            int i = ii % tmp.size();
            auto first = tmp[i];
            auto mid_index = (i + 1) % tmp.size();
            auto mid = tmp[mid_index];
            auto last = tmp[(i + 2) % tmp.size()];
            float angle = angleAround(first, mid, last);
            if (angle < 0.f) {
                continue;
            }
            bool containsVertex = false;
            for (auto p : tmp) {
                if (p == first || p == mid || p == last)
                    continue;
                if (isOverlappingPointPoly(p, {first, mid, last})) {
                    containsVertex = true;
                    break;
                }
            }
            if (containsVertex) {
                continue;
            }
            result.push_back({last, mid, first});
            expandedResult = true;
            tmp.erase(tmp.begin() + mid_index);
            break;
        }
        if (!expandedResult) {
            break;
        }
    }
    return result;
}
std::vector<Triangle> triangulate(const std::vector<vec2f>& points) {
    return triangulateEarClipping<Triangle>(points);
}
std::vector<std::vector<vec2f>> triangulateAsVector(
        const std::vector<vec2f>& points
) {
    return triangulateEarClipping<std::vector<vec2f>>(points);
}
bool hasSharedEdge(
        std::vector<vec2f> points0, const std::vector<vec2f>& points1
) {
    std::reverse(points0.begin(), points0.end());

    auto last0 = points0.back();
    for (auto vert0 : points0) {
        auto last1 = points1.back();
        for (auto vert1 : points1) {
            if (vert0 == vert1 && last0 == last1)
                return true;
            last1 = vert1;
        }
        last0 = vert0;
    }
    return false;
}
std::pair<bool, std::vector<vec2f>> static tryMergingToConvex(
        vec2f avg,
        const std::vector<vec2f>& points0,
        const std::vector<vec2f>& points1
) {
    std::vector<vec2f> point_merge = points0;
    point_merge.insert(point_merge.end(), points1.begin(), points1.end());
    for (auto& p : point_merge)
        p -= avg;
    std::sort(point_merge.begin(), point_merge.end(), [](vec2f a, vec2f b) {
        return atan2(a.y, a.x) > atan2(b.y, b.x);
    });
    for (auto& p : point_merge)
        p += avg;
    for (int i = 0; i < point_merge.size() - 1; i++) {
        if (point_merge[i] == point_merge[i + 1]) {
            point_merge.erase(point_merge.begin() + i);
            i--;
        }
    }

    for (int i = 0; i < point_merge.size(); i++) {
        auto first = point_merge[i];
        auto mid = point_merge[(i + 1) % point_merge.size()];
        auto last = point_merge[(i + 2) % point_merge.size()];
        if (angleAround(first, mid, last) < 0.f)
            return {false, {}};
    }
    return {true, point_merge};
}
static std::vector<std::vector<vec2f>> partitionConvexDIY(
        const std::vector<std::vector<vec2f>>& polygons
) {
    std::vector<bool> wasIncluded(polygons.size(), false);
    std::vector<std::vector<vec2f>> result;
    for (int i = 0; i < polygons.size(); i++) {
        if (wasIncluded[i])
            continue;
        ConvexPolygon current = ConvexPolygon::CreateFromPoints(polygons[i]);
        wasIncluded[i] = true;
        bool wasFound = true;
        while (wasFound) {
            wasFound = false;
            for (int ii = i + 1; ii < polygons.size(); ii++) {
                if (wasIncluded[ii])
                    continue;
                if (!hasSharedEdge(current.getVertecies(), polygons[ii]))
                    continue;
                auto convexMerge = tryMergingToConvex(
                        current.getPos(), current.getVertecies(), polygons[ii]
                );
                if (!convexMerge.first)
                    continue;
                current = ConvexPolygon::CreateFromPoints(convexMerge.second);
                wasIncluded[ii] = true;
                wasFound = true;
                break;
            }
        }
        result.push_back(current.getVertecies());
    }
    return result;
}
static bool isConvex(vec2f p1, vec2f p2, vec2f p3) {
    float tmp;
    tmp = (p3.y - p1.y) * (p2.x - p1.x) - (p3.x - p1.x) * (p2.y - p1.y);
    if (tmp > 0) {
        return 1;
    } else {
        return 0;
    }
}

static std::vector<ConvexPolygon> partitionConvexHertelMehlhornPoly(
        std::vector<ConvexPolygon> triangles
) {
    vec2f d1, d2, p1, p2, p3;
    bool isdiagonal;
    long i11, i12, i21, i22, i13, i23, j, k;
    for (auto iter1 = triangles.begin(); iter1 != triangles.end(); iter1++) {
        auto poly1 = &(*iter1);
        decltype(poly1) poly2;
        decltype(iter1) iter2;

        for (i11 = 0; i11 < poly1->getVertecies().size(); i11++) {
            auto d1 = poly1->getVertecies()[i11];
            i12 = (i11 + 1) % (poly1->getVertecies().size());
            d2 = poly1->getVertecies()[i12];
            isdiagonal = false;
            for (iter2 = iter1; iter2 != triangles.end(); iter2++) {
                if (iter1 == iter2) {
                    continue;
                }
                poly2 = &(*iter2);

                for (i21 = 0; i21 < poly2->getVertecies().size(); i21++) {
                    if ((d2.x != poly2->getVertecies()[i21].x) ||
                        (d2.y != poly2->getVertecies()[i21].y)) {
                        continue;
                    }
                    i22 = (i21 + 1) % (poly2->getVertecies().size());
                    if ((d1.x != poly2->getVertecies()[i22].x) ||
                        (d1.y != poly2->getVertecies()[i22].y)) {
                        continue;
                    }
                    isdiagonal = true;
                    break;
                }
                if (isdiagonal) {
                    break;
                }
            }
            if (!isdiagonal) {
                continue;
            }

            p2 = poly1->getVertecies()[i11];
            if (i11 == 0) {
                i13 = poly1->getVertecies().size() - 1;
            } else {
                i13 = i11 - 1;
            }
            p1 = poly1->getVertecies()[i13];
            if (i22 == (poly2->getVertecies().size() - 1)) {
                i23 = 0;
            } else {
                i23 = i22 + 1;
            }
            p3 = poly2->getVertecies()[i23];

            if (!isConvex(p1, p2, p3)) {
                continue;
            }

            p2 = poly1->getVertecies()[i12];
            if (i12 == (poly1->getVertecies().size() - 1)) {
                i13 = 0;
            } else {
                i13 = i12 + 1;
            }
            p3 = poly1->getVertecies()[i13];
            if (i21 == 0) {
                i23 = poly2->getVertecies().size() - 1;
            } else {
                i23 = i21 - 1;
            }
            p1 = poly2->getVertecies()[i23];

            if (!isConvex(p1, p2, p3)) {
                continue;
            }

            std::vector<vec2f> newpoly;
            for (j = i12; j != i11;
                 j = (j + 1) % (poly1->getVertecies().size())) {
                newpoly.push_back(poly1->getVertecies()[j]);
            }
            for (j = i22; j != i21;
                 j = (j + 1) % (poly2->getVertecies().size())) {
                newpoly.push_back(poly2->getVertecies()[j]);
            }
            triangles.erase(iter2);
            *iter1 = ConvexPolygon::CreateFromPoints(newpoly);
            poly1 = &(*iter1);
            i11 = -1;
        }
    }
    return triangles;
}
static std::vector<std::vector<vec2f>> partitionConvexHertelMehlhorn(
        std::vector<std::vector<vec2f>> result
) {
    vec2f d1, d2, p1, p2, p3;
    bool isdiagonal;
    long i11, i12, i21, i22, i13, i23, j, k;
    for (auto iter1 = result.begin(); iter1 != result.end(); iter1++) {
        auto poly1 = &(*iter1);
        decltype(poly1) poly2;
        decltype(iter1) iter2;

        for (i11 = 0; i11 < poly1->size(); i11++) {
            auto d1 = (*poly1)[i11];
            i12 = (i11 + 1) % (poly1->size());
            d2 = (*poly1)[i12];
            isdiagonal = false;
            for (iter2 = iter1; iter2 != result.end(); iter2++) {
                if (iter1 == iter2) {
                    continue;
                }
                poly2 = &(*iter2);

                for (i21 = 0; i21 < poly2->size(); i21++) {
                    if ((d2.x != (*poly2)[i21].x) ||
                        (d2.y != (*poly2)[i21].y)) {
                        continue;
                    }
                    i22 = (i21 + 1) % (poly2->size());
                    if ((d1.x != (*poly2)[i22].x) ||
                        (d1.y != (*poly2)[i22].y)) {
                        continue;
                    }
                    isdiagonal = true;
                    break;
                }
                if (isdiagonal) {
                    break;
                }
            }
            if (!isdiagonal) {
                continue;
            }

            p2 = (*poly1)[i11];
            if (i11 == 0) {
                i13 = poly1->size() - 1;
            } else {
                i13 = i11 - 1;
            }
            p1 = (*poly1)[i13];
            if (i22 == (poly2->size() - 1)) {
                i23 = 0;
            } else {
                i23 = i22 + 1;
            }
            p3 = (*poly2)[i23];

            if (!isConvex(p1, p2, p3)) {
                continue;
            }

            p2 = (*poly1)[i12];
            if (i12 == (poly1->size() - 1)) {
                i13 = 0;
            } else {
                i13 = i12 + 1;
            }
            p3 = (*poly1)[i13];
            if (i21 == 0) {
                i23 = poly2->size() - 1;
            } else {
                i23 = i21 - 1;
            }
            p1 = (*poly2)[i23];

            if (!isConvex(p1, p2, p3)) {
                continue;
            }

            std::vector<vec2f> newpoly;
            for (j = i12; j != i11; j = (j + 1) % (poly1->size())) {
                newpoly.push_back((*poly1)[j]);
            }
            for (j = i22; j != i21; j = (j + 1) % (poly2->size())) {
                newpoly.push_back((*poly2)[j]);
            }
            result.erase(iter2);
            *iter1 = newpoly;
            poly1 = &(*iter1);
            i11 = -1;
        }
    }
    return result;
}
std::vector<std::vector<vec2f>> mergeToConvex(
        const std::vector<std::vector<vec2f>>& polygons
) {
    return partitionConvexHertelMehlhorn(polygons);
}
float calcTriangleVolume(vec2f a, vec2f b, vec2f c) {
    auto la = length(a - b);
    auto lb = length(b - c);
    auto lc = length(c - a);
    auto S = la + lb + lc;
    S *= 0.5f;
    return sqrt(S * (S - la) * (S - lb) * (S - lc));
}
#define SQR(x) ((x) * (x))
bool isOverlappingPointAABB(const vec2f& p, const AABB& r) {
    return (p.x >= r.center().x - r.size().x / 2 &&
            p.y > r.center().y - r.size().y / 2 &&
            p.x < r.center().x + r.size().x / 2 &&
            p.y <= r.center().y + r.size().y / 2);
}
bool isOverlappingPointCircle(const vec2f& p, const Circle& c) {
    return length(p - c.pos) <= c.radius;
}
bool isOverlappingPointPoly(const vec2f& p, const std::vector<vec2f>& points) {
    int i, j, c = 0;
    for (i = 0, j = points.size() - 1; i < points.size(); j = i++) {
        auto& vi = points[i];
        auto& vj = points[j];
        if (((vi.y > p.y) != (vj.y > p.y)) &&
            (p.x < (vj.x - vi.x) * (p.y - vi.y) / (vj.y - vi.y) + vi.x))
            c = !c;
    }
    return c;
}
bool isOverlappingAABBAABB(const AABB& r1, const AABB& r2) {
    return (r1.min.x <= r2.max.x && r1.max.x >= r2.min.x &&
            r1.min.y <= r2.max.y && r1.max.y >= r2.min.y);
}
bool AABBcontainsAABB(const AABB& r1, const AABB& r2) {
    return (r2.min.x >= r1.min.x) && (r2.max.x <= r1.max.x) &&
           (r2.min.y >= r1.min.y) && (r2.max.y <= r1.max.y);
}
IntersectionRayAABBResult intersectRayAABB(
        vec2f ray_origin, vec2f ray_dir, const AABB& target
) {
    IntersectionRayAABBResult result;
    vec2f invdir = {1.0f / ray_dir.x, 1.0f / ray_dir.y};
    vec2f t_size = target.size();
    // VVVVVVVVVVVVV
    // if((int)target.size.y % 2 == 0 && target.pos.y > ray_origin.y)
    // t_size -= vec2f(0, 1);
    //^^^^^^^^^^^^^
    vec2f t_near = (target.center() - t_size / 2.f - ray_origin) * invdir;
    vec2f t_far = (target.center() + t_size / 2.f - ray_origin) * invdir;

    if (std::isnan(t_far.y) || std::isnan(t_far.x))
        return {false};
    if (std::isnan(t_near.y) || std::isnan(t_near.x))
        return {false};
    if (t_near.x > t_far.x)
        std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y)
        std::swap(t_near.y, t_far.y);

    if (t_near.x > t_far.y || t_near.y > t_far.x)
        return {false};
    float t_hit_near = std::max(t_near.x, t_near.y);
    result.time_hit_near = t_hit_near;
    float t_hit_far = std::min(t_far.x, t_far.y);
    result.time_hit_far = t_hit_far;

    if (t_hit_far < 0)
        return {false};
    result.contact_point = ray_origin + ray_dir * t_hit_near;
    if (t_near.x > t_near.y) {
        if (invdir.x < 0)
            result.contact_normal = {1, 0};
        else
            result.contact_normal = {-1, 0};
    } else if (t_near.x < t_near.y) {
        if (invdir.y < 0)
            result.contact_normal = {0, 1};
        else
            result.contact_normal = {0, -1};
    }
    result.detected = true;
    return result;
}
IntersectionRayRayResult intersectRayRay(
        vec2f ray0_origin, vec2f ray0_dir, vec2f ray1_origin, vec2f ray1_dir
) {
    if (ray0_origin == ray1_origin) {
        return {true, false, ray0_origin, 0.f, 0.f};
    }
    auto dx = ray1_origin.x - ray0_origin.x;
    auto dy = ray1_origin.y - ray0_origin.y;
    float det = ray1_dir.x * ray0_dir.y - ray1_dir.y * ray0_dir.x;
    if (!nearlyEqual(det, 0)) { // near parallel line will yield noisy results
        float u = (dy * ray1_dir.x - dx * ray1_dir.y) / det;
        float v = (dy * ray0_dir.x - dx * ray0_dir.y) / det;
        return {u >= 0 && v >= 0 && u <= 1.f && v <= 1.f,
                false,
                ray0_origin + ray0_dir * u,
                u,
                v};
    }
    return {false, true};
}
/*
    bool detected;
    vec2f contact_point;
    float t_hit_near0;
    float t_hit_near1;
*/
IntersectionRayPolygonResult intersectRayPolygon(
        vec2f ray_origin, vec2f ray_dir, const ConvexPolygon& poly
) {
    for (auto v : poly.getVertecies()) {
        auto intersection = intersectRayRay(
                ray_origin, ray_dir, poly.getPos(), v - poly.getPos()
        );
        if (intersection.detected) {
            auto closest = findClosestPointOnRay(ray_origin, ray_dir, v);
            return {true, normal(closest - v), closest, qlen(v - closest)};
        }
    }
    return {false};
}
vec2f findClosestPointOnRay(vec2f ray_origin, vec2f ray_dir, vec2f point) {
    float ray_dir_len = length(ray_dir);
    vec2f ray_dir_normal = ray_dir / ray_dir_len;
    float proj = dot(point - ray_origin, ray_dir_normal);
    if (proj <= 0)
        return ray_origin;
    if (proj >= ray_dir_len)
        return ray_origin + ray_dir;
    return ray_dir_normal * proj + ray_origin;
}
vec2f findClosestPointOnEdge(vec2f point, const std::vector<vec2f>& points) {
    vec2f closest(INFINITY, INFINITY);
    float closest_dist = INFINITY;
    for (size_t i = 0; i < points.size(); i++) {
        vec2f a = points[i];
        vec2f b = points[(i + 1) % points.size()];
        vec2f adir = b - a;
        vec2f t = findClosestPointOnRay(a, adir, point);
        float dist = length(t - point);
        if (dist < closest_dist) {
            closest_dist = dist;
            closest = t;
        }
    }
    return closest;
}
std::vector<vec2f> findContactPointFast(
        const ConvexPolygon* p0, const ConvexPolygon* p1, vec2f cn
) {
    float best = 0.f;
    std::vector<vec2f> contact_point;
    std::swap(p0, p1);
    for (int i = 0; i < 2; i++) {
        if (i == 1) {
            std::swap(p0, p1);
            cn *= -1.f;
        }
        for (auto p : p0->getVertecies()) {
            auto d = dot(normal(p - p0->getPos()), cn);
            if (abs(best - d) != 0.f) {
                best = d;
                contact_point.push_back(p);
            } else if (best > d) {
                best = d;
                contact_point = {p};
            }
        }
    }
    return {contact_point};
}

std::vector<vec2f> findContactPoints(
        const std::vector<vec2f>& p0, const std::vector<vec2f>& p1
) {
    std::vector<vec2f> result;
    const std::vector<vec2f>* poly[] = {&p0, &p1};
    struct Seg {
        char polyID;
        float x_pos;
        size_t segID;
        bool isEnding;
        // if isEnding then x_start_pos is the beggining of ray else ray is the
        // struct
        Ray ray;
    };
    std::vector<Seg> all;
    std::vector<Seg> open[2];
    all.reserve(p1.size() * 2 + p0.size() * 2);
    open[0].reserve(p0.size());
    open[1].reserve(p1.size());
    size_t cur_id = 0;
    for (char i = 0; i < 2; i++) {
        auto prev = poly[i]->back();
        for (auto p : *poly[i]) {
            auto mi = std::min(prev.x, p.x);
            auto mx = std::max(prev.x, p.x);
            all.push_back({i, mi, cur_id, false, Ray::CreatePoints(prev, p)});
            all.push_back({i, mx, cur_id, true, Ray::CreatePoints(prev, p)});
            cur_id += 1;
            prev = p;
        }
    }
    std::sort(
            all.begin(),
            all.end(),
            [&](const Seg& s1, const Seg& s2) -> bool {
                return s1.x_pos < s2.x_pos;
            }
    );
    for (auto a : all) {
        if (!a.isEnding) {
            for (auto p : open[!a.polyID]) {
                auto intersection = intersectRayRay(
                        p.ray.pos, p.ray.dir, a.ray.pos, a.ray.dir
                );
                if (intersection.detected) {
                    result.push_back(intersection.contact_point);
                }
            }
            open[a.polyID].push_back(a);
        } else {
            auto itr = std::find_if(
                    open[a.polyID].begin(),
                    open[a.polyID].end(),
                    [&](const Seg& s1) { return s1.segID == a.segID; }
            );
            if (itr == open[a.polyID].end()) {
                std::cerr << "tried to find a line segment that is not present";
                continue;
            }
            open[a.polyID].erase(itr);
        }
    }
    return result;
}
std::vector<vec2f> findContactPoints(
        const ConvexPolygon& p0, const ConvexPolygon& p1
) {
    return findContactPoints(p0.getVertecies(), p1.getVertecies());
}
vec2f centerOfMass(std::vector<vec2f> model) {
    vec2f inside = std::reduce(model.begin(), model.end()) /
                   static_cast<float>(model.size());
    vec2f sum_avg = {0, 0};
    float sum_weight = 0.f;
    auto prev = model.back();
    for (auto next : model) {
        auto a = prev - inside;
        auto b = next - inside;
        float area_step = abs(perp_dot(a, b)) / 2.f;
        sum_weight += area_step;
        sum_avg += (prev + next + inside) / 3.f * area_step;
        prev = next;
    }
    return sum_avg / sum_weight;
}
float area(const std::vector<vec2f>& model) {
    double area = 0.0;
    // Calculate value of shoelace formula
    for (int i = 0; i < model.size(); i++) {
        int i1 = (i + 1) % model.size();
        area += (model[i].y + model[i1].y) * (model[i1].x - model[i].x) / 2.0;
    }
    return abs(area / 2.0);
}
// std::pair<float, float> calcProjectionPolygon(const std::vector<vec2f>&r,
// vec2f projectionAxis) {
// }
struct IntersetionPolygonPolygonAxis {
    bool detected = false;
    float overlap = INFINITY;
    vec2f cn;
    bool foundSeparatingAxis = false;
    vec2f most_pertruding;
};
template <class Key, class Data>
std::pair<Key, Data> compMax(std::pair<Key, Data> p1, std::pair<Key, Data> p2) {
    return (p1.first > p2.first ? p1 : p2);
}
template <class Key, class Data>
std::pair<Key, Data> compMin(std::pair<Key, Data> p1, std::pair<Key, Data> p2) {
    return (p1.first < p2.first ? p1 : p2);
}
IntersectionPolygonPolygonResult intersectPolygonPolygon(
        const std::vector<vec2f>& r1, const std::vector<vec2f>& r2
) {
    float overlap = INFINITY;
    vec2f cn;

    int incidentEdgeOwner = -1;
    std::pair<vec2f, vec2f> incident_edge;
    vec2f contact_vertex;

    for (int flipShapes = 0; flipShapes < 2; flipShapes++) {
        auto& poly1 = (flipShapes ? r2 : r1);
        auto& poly2 = (flipShapes ? r1 : r2);
        vec2f contact_vertex_p1;
        vec2f contact_vertex_p2;
        for (int a = 0; a < poly1.size(); a++) {
            int b = (a + 1) % poly1.size();
            vec2f axis_proj_perp = poly1[b] - poly1[a];
            vec2f axis_proj =
                    normal(vec2f(-axis_proj_perp.y, axis_proj_perp.x));

            // very rare TODO: remove findClosestPointONRay from inner loop
            if (incidentEdgeOwner == flipShapes &&
                (cn == axis_proj || cn == -axis_proj)) {
                auto closest_current = findClosestPointOnRay(
                        incident_edge.first,
                        incident_edge.second - incident_edge.first,
                        contact_vertex
                );
                auto closest_potential = findClosestPointOnRay(
                        poly1[a], poly1[b] - poly1[a], contact_vertex
                );
                if (qlen(closest_current - contact_vertex) >
                    qlen(closest_potential - contact_vertex)) {
                    incidentEdgeOwner = flipShapes;
                    incident_edge = {poly1[a], poly1[b]};
                }
            }

            std::pair<float, vec2f> min_r1 = {INFINITY, {}},
                                    max_r1 = {-INFINITY, {}};
            for (int p = 0; p < poly1.size(); p++) {
                float q = dot(poly1[p], axis_proj);
                min_r1 = compMin(min_r1, {q, poly1[p]});
                max_r1 = compMax(max_r1, {q, poly1[p]});
            }

            std::pair<float, vec2f> min_r2 = {INFINITY, {}},
                                    max_r2 = {-INFINITY, {}};
            for (int p = 0; p < poly2.size(); p++) {
                float q = dot(poly2[p], axis_proj);
                min_r2 = compMin(min_r2, {q, poly2[p]});
                max_r2 = compMax(max_r2, {q, poly2[p]});
            }

            auto minmax = compMin(max_r1, max_r2).first;
            auto maxmin = compMax(min_r1, min_r2).first;
            auto current_overlap = minmax - maxmin;
            if (current_overlap < overlap) {
                overlap = current_overlap;
                cn = axis_proj;
                incidentEdgeOwner = flipShapes;
                incident_edge.first = poly1[a];
                incident_edge.second = poly1[b];
                contact_vertex =
                        (minmax == max_r2.first ? max_r2.second : min_r2.second
                        );

                if ((minmax != max_r2.first) ^ flipShapes) {
                    cn *= -1.f;
                }
            }

            if (!(max_r2.first >= min_r1.first && max_r1.first >= min_r2.first))
                return {false};
        }
    }
    if (overlap == 0.f)
        return {false};

    auto closest = findClosestPointOnRay(
            incident_edge.first,
            incident_edge.second - incident_edge.first,
            contact_vertex
    );
    auto cp1 = (incidentEdgeOwner ? contact_vertex : closest);
    auto cp2 = (incidentEdgeOwner ? closest : contact_vertex);
    return {true, cn, overlap, cp1, cp2};
}
IntersectionPolygonPolygonResult intersectPolygonPolygon(
        const ConvexPolygon& r1, const ConvexPolygon& r2
) {
    return intersectPolygonPolygon(r1.getVertecies(), r2.getVertecies());
}
IntersectionPolygonCircleResult intersectCirclePolygon(
        const Circle& c, const ConvexPolygon& r
) {
    vec2f max_reach = c.pos + normal(r.getPos() - c.pos) * c.radius;

    vec2f cn;
    vec2f closest(INFINITY, INFINITY);
    vec2f prev = r.getVertecies().back();
    for (const auto& p : r.getVertecies()) {
        vec2f tmp = findClosestPointOnRay(prev, p - prev, c.pos);
        if (qlen(closest - c.pos) > qlen(tmp - c.pos)) {
            closest = tmp;
        }
        prev = p;
    }
    bool isOverlappingPoint = isOverlappingPointPoly(c.pos, r.getVertecies());
    bool isOverlapping =
            qlen(closest - c.pos) <= c.radius * c.radius || isOverlappingPoint;
    if (!isOverlapping) {
        return {false};
    }
    cn = normal(c.pos - closest);
    //    //correcting normal
    //    if(dot(cn, r.getPos() - closest) > 0.f) {
    //        cn *= -1.f;
    //    }
    float l = length(c.pos - closest);
    float overlap = c.radius - l;
    if (isOverlappingPoint) {
        overlap = l + c.radius;
        cn *= -1.f;
    }
    return {true, cn, closest, overlap};
}
IntersectionCircleCircleResult intersectCircleCircle(
        const Circle& c1, const Circle& c2
) {
    vec2f dist = c1.pos - c2.pos;
    float dist_len = length(dist);
    if (dist_len > c1.radius + c2.radius) {
        return {false};
    }
    float overlap = c1.radius + c2.radius - dist_len;
    vec2f contact_point = dist / dist_len * c2.radius + c2.pos;
    return {true, dist / dist_len, contact_point, overlap};
}
float calculateInertia(const std::vector<vec2f>& model, float mass) {
    float area = 0;
    float mmoi = 0;

    auto prev = model.back();
    for (auto next : model) {
        float area_step = abs(perp_dot(prev, next)) / 2.f;
        float mmoi_step =
                area_step *
                (dot(prev, prev) + dot(next, next) + abs(dot(prev, next))) /
                6.f;

        area += area_step;
        mmoi += mmoi_step;

        prev = next;
    }

    double density = mass / area;
    mmoi *= density;
    // mmoi -= mass * dot(center, center);
    if (std::isnan(mmoi)) {
        std::cerr << "mmoi calc erreor!";
        mmoi = 0.f;
    }
    return abs(mmoi);
}

MIAInfo calculateMassInertiaArea(
        const std::vector<vec2f>& model, float thickness, float density
) {
    const auto triangles = triangulate(model);
    std::vector<float> mass(triangles.size(), 0.f);
    std::vector<float> mmoi(triangles.size(), 0.f);
    std::vector<vec2f> centroid(triangles.size(), {0.f, 0.f});
    float combinedMass = 0;
    float combinedMMOI = 0;
    float combinedArea = 0;
    vec2f combinedCentroid(0, 0);
    for (int i = 0; i < triangles.size(); ++i) {
        mass[i] = triangles[i].calcMass(thickness, density);
        mmoi[i] = triangles[i].calcMMOI(mass[i]);
        centroid[i] = triangles[i].calcCentroid();

        combinedArea += triangles[i].calcArea();
        combinedMass += mass[i];
        combinedCentroid += mass[i] * centroid[i];
    }
    combinedCentroid /= combinedMass;

    for (int i = 0; i < triangles.size(); ++i) {
        combinedMMOI +=
                mmoi[i] + mass[i] * qlen(centroid[i] - combinedCentroid);
    }
    return {combinedMMOI, combinedMass, combinedArea, combinedCentroid};
}
} // namespace emp
