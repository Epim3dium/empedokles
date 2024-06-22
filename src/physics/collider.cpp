#include "collider.hpp"
#include "math/geometry_func.hpp"
namespace emp {
    Collider::Collider(std::vector<vec2f> shape, emp::Transform* trans, bool correctCOM) {
        auto MIA = calculateMassInertiaArea(shape);
        m_inertia_dev_mass = MIA.MMOI;
        m_area = MIA.area;
        if(correctCOM) {
            for(auto& p : shape) {
                p -= MIA.centroid;
            }
        }
        m_model_outline = shape;
        m_transformed_outline = m_model_outline;

        auto triangles = triangulateAsVector(shape);
        m_model_shape = mergeToConvex(triangles);
        m_transformed_shape = m_model_shape;
        m_transform = trans;
    }
    void Collider::update() {
        if(m_transform == nullptr) {
            m_transformed_outline = m_model_outline;
            m_transformed_shape = m_model_shape;
            return;
        }

        for(int i = 0; i < m_model_shape.size(); i++) {
            auto& poly = m_model_shape[i];
            for(int ii = 0; ii < poly.size(); ii++) {
                m_transformed_shape[i][ii] = m_transform->globalTransform().transformPoint(poly[ii]);
            }
        }
        for (int i = 0; i < m_model_outline.size(); i++) {
            m_transformed_outline[i] = m_transform->globalTransform().transformPoint(m_model_outline[i]);
        }
    }
};
