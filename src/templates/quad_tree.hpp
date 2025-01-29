#ifndef EMP_QUAD_TREE_HPP
#define EMP_QUAD_TREE_HPP
#include <functional>
#include <map>
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
#include "templates/free_list.hpp"
namespace emp {
template<typename T, typename GetAABB, typename Equal = std::equal_to<T>, typename Float = float>
class QuadTree
{
    static_assert(std::is_convertible_v<std::invoke_result_t<GetAABB, const T&>, AABB>,
        "GetAABB must be a callable of signature AABB<Float>(const T&)");
    static_assert(std::is_convertible_v<std::invoke_result_t<Equal, const T&, const T&>, bool>,
        "Equal must be a callable of signature bool(const T&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

public:
    QuadTree(const AABB& box, const GetAABB& getAABB = GetAABB(),
        const Equal& equal = Equal()) :
        m_box(box), m_getAABB(getAABB), m_equal(equal)
    { 
        m_nodes.insert({-1});
    }

    void add(const T& value)
    {
        add(m_root, 0, m_box, value);
    }

    void remove(const T& value)
    {
        remove(m_root, m_box, value);
    }

    std::vector<T> query(const AABB& box) const
    {
        auto values = std::vector<T>();
        query(m_root, m_box, box, values);
        return values;
    }
    void update(T value) {
        // auto itr = _locations.find(value);
        // if(itr != _locations.end()) {
        //     auto b = itr->second->box;
        //     if(AABBcontainsAABB(b, _getAABB(value))) 
        //         return;
        //     remove(value);
        //     add(value);
        //
        // } else {
        //     add(value);
        // }
    }


    std::vector<std::pair<T, T>> findAllIntersections() const
    {
        return findAllIntersections(m_root);
    }

    AABB getAABB() const 
    {
        return m_box;
    }
    void updateLeafes() {
        updateLeafes(m_root);
    }
    void clear() {
        clear(m_root);

    }
    
private:
    static constexpr size_t threshold = 8;
    static constexpr size_t max_depth = 8;

    struct Node
    {
        int first_child = -1;
        std::vector<T> values;
    };

    AABB m_box;
    static constexpr int m_root = 0U;
    FreeList<Node> m_nodes;

    GetAABB m_getAABB;
    Equal m_equal;

    bool isLeaf(int node) const
    {
        return (m_nodes[node].first_child == -1);
    }

    AABB computeAABB(const AABB& box, int i) const
    {
        auto origin = box.min;
        auto childSize = box.size() / static_cast<Float>(2);
        switch (i)
        {
            case 0:
                return AABB::CreateMinSize(origin, childSize);
            case 1:
                return AABB::CreateMinSize(vec2f(origin.x + childSize.x, origin.y), childSize);
            case 2:
                return AABB::CreateMinSize(vec2f(origin.x, origin.y + childSize.y), childSize);
            case 3:
                return AABB::CreateMinSize(origin + childSize, childSize);
            default:
                assert(false && "Invalid child index");
                return AABB();
        }
    }

    int getQuadrant(const AABB& nodeAABB, const AABB& valueAABB) const
    {
        auto center = nodeAABB.center();
        if (valueAABB.right() < center.x)
        {
            if (valueAABB.top() < center.y)
                return 0;
            else if (valueAABB.bottom() >= center.y)
                return 2;
            else
                return -1;
        }
        else if (valueAABB.left() >= center.x)
        {
            if (valueAABB.top() < center.y)
                return 1;
            else if (valueAABB.bottom() >= center.y)
                return 3;
            else
                return -1;
        }
        else
            return -1;
    }

    int add(const int node_idx, size_t depth, const AABB& box, const T& value)
    {
        assert(node_idx != -1);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if (isLeaf(node_idx))
        {
            if (depth >= max_depth || m_nodes[node_idx].values.size() < threshold) {
                m_nodes[node_idx].values.push_back(value);
                return node_idx;
            } else {
                split(node_idx, box);
                return add(node_idx, depth, box, value);
            }
        } 
        auto quadrant = getQuadrant(box, m_getAABB(value));
        if (quadrant != -1) {
            int child_idx = m_nodes[node_idx].first_child + quadrant;
            return add(child_idx, depth + 1, computeAABB(box, quadrant), value);
        } else {
            m_nodes[node_idx].values.push_back(value);
            return node_idx;
        }
        exit(1);
    }

    void split(int node_idx, const AABB& box)
    {
        assert(node_idx != -1);
        assert(isLeaf(node_idx) && "Only leaves can be split");
        int quad = 0;
        int first_child = 0xffffff;
        for(int i = 0; i < 4; i++) {
            auto new_child = m_nodes.insert({-1});
            first_child = std::min(new_child, first_child);
        }
        m_nodes[node_idx].first_child = first_child;
        auto newValues = std::vector<T>(); 
        for (const auto& value : m_nodes[node_idx].values)
        {
            auto quadrant = getQuadrant(box, m_getAABB(value));
            if (quadrant != -1) {
                int child_idx = quadrant + m_nodes[node_idx].first_child;
                m_nodes[child_idx].values.push_back(value);
            } else {
                newValues.push_back(value);
            }
        }
        m_nodes[node_idx].values = std::move(newValues);
    }

    bool remove(int node_idx, const AABB& box, const T& value)
    {
        assert(node_idx != -1);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if (isLeaf(node_idx))
        {
            removeValue(node_idx, value);
            return true;
        }
        else
        {
            auto quadrant = getQuadrant(box, m_getAABB(value));
            if (quadrant != -1)
            {
                int child_idx = m_nodes[node_idx].first_child + quadrant;
                return remove(child_idx, computeAABB(box, quadrant), value);
            }
            else
                removeValue(node_idx, value);
            return false;
        }
    }

    void removeValue(int node_idx, const T& value)
    {
        auto it = std::find_if(std::begin(m_nodes[node_idx].values), std::end(m_nodes[node_idx].values),
            [this, &value](const auto& rhs){ return m_equal(value, rhs); });
        assert(it != std::end(m_nodes[node_idx].values) && "Trying to remove a value that is not present in the node");
        *it = std::move(m_nodes[node_idx].values.back());
        m_nodes[node_idx].values.pop_back();
    }

    bool tryMerge(int node_idx)
    {
        assert(node_idx != -1);
        assert(!isLeaf(node_idx) && "Only interior nodes can be merged");
        auto nbValues = m_nodes[node_idx].values.size();
        for (int i = 0; i < 4; i++)
        {
            int child_idx = m_nodes[node_idx].first_child + i;
            if (!isLeaf(child_idx))
                return false;
            nbValues += m_nodes[child_idx].values.size();
        }
        if (nbValues <= threshold)
        {
            m_nodes[node_idx].values.reserve(nbValues);
            for (int i = 0; i < 4; i++)
            {
                int child_idx = m_nodes[node_idx].first_child + i;
                for (const auto& value : m_nodes[child_idx].values) {
                    m_nodes[node_idx].values.push_back(value);
                }
            }
            for (int i = 0; i < 4; i++)
            {
                int child_idx = m_nodes[node_idx].first_child + i;
                m_nodes.erase(child_idx);
            }
            m_nodes[node_idx].first_child = -1;
            return true;
        }
        else
            return false;
    }
    void updateLeafes(int node_idx) {
        auto isEndBranch = isLeaf(node_idx) || tryMerge(node_idx); 
        if(isEndBranch)
            return;
        auto first = m_nodes[node_idx].first_child;
        for (int i = 0; i < 4; i++)
        {
            int child_idx = m_nodes[node_idx].first_child + i;
            if(child_idx == node_idx)
                exit(0);
            updateLeafes(child_idx);
        }
    }
    void clear(int node_idx) {
        if(node_idx == -1)
            return;
        m_nodes[node_idx].values.clear();
        if(isLeaf(node_idx))
            return;
        for (int i = 0; i < 4; i++)
        {
            int child_idx = m_nodes[node_idx].first_child + i;
            clear(child_idx);
        }
    }

    void query(int node_idx, const AABB& box, const AABB& queryAABB, std::vector<T>& values) const
    {
        assert(node_idx != -1);
        if(!isOverlappingAABBAABB(queryAABB, box)) {
            return;
        }
        for (const auto& value : m_nodes[node_idx].values) {
            if (isOverlappingAABBAABB(queryAABB, m_getAABB(value)))
                values.push_back(value);
        }
        if (!isLeaf(node_idx)) {
            for (int i = 0; i < 4; i++) {
                auto childAABB = computeAABB(box, static_cast<int>(i));
                auto child_idx = m_nodes[node_idx].first_child + i;
                if (isOverlappingAABBAABB(queryAABB, childAABB))
                    query(child_idx, childAABB, queryAABB, values);
            }
        }
    }

    void searchIntersecionsInNode(int node_idx, int depth,
        std::vector<std::pair<T, T>>& intersections,
        std::array<int, max_depth>& parent_stack) const
    {
        if(node_idx == -1)
            return;
        auto& values = m_nodes[node_idx].values;
        for (size_t i = 0; i < values.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                if (isOverlappingAABBAABB(m_getAABB(values[i]), m_getAABB(values[j])))
                    intersections.emplace_back(values[i], values[j]);
            }
        }
        for(int deep = 0; deep < depth; deep++) {
            auto parent_idx = parent_stack[deep];
            assert(parent_idx != -1);
            auto& parent_values = m_nodes[parent_idx].values;
            for (size_t i = 0; i < values.size(); ++i) {
                for (size_t j = 0; j < parent_values.size(); ++j) {
                    if (isOverlappingAABBAABB(m_getAABB(values[i]), m_getAABB(parent_values[j])))
                        intersections.emplace_back(values[i], parent_values[j]);
                }
            }
        }
    }
    std::vector<std::pair<T, T>> findAllIntersections(int root_idx) const 
    {
        std::vector<std::pair<T, T>> intersections;

        std::stack<std::pair<int, int>> to_process;
        std::array<int, max_depth> parent_stack;
        to_process.push({root_idx, 0});
        while(!to_process.empty()) {
            auto node_info = to_process.top();
            to_process.pop();
            if(node_info.first == -1)
                continue;;
            searchIntersecionsInNode(node_info.first, node_info.second, intersections, parent_stack);
            if(isLeaf(node_info.first))
                continue;
            parent_stack[node_info.second] = node_info.first;
            if(isLeaf(node_info.first))
                continue;
            int first_child_idx = m_nodes[node_info.first].first_child;
            for (int i = 0; i < 4; i ++) {
                 to_process.push({first_child_idx + i, node_info.second + 1});
            }
        }
        return intersections;
    }
};

}
#endif
