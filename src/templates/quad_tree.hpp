#ifndef EMP_QUAD_TREE_HPP
#define EMP_QUAD_TREE_HPP
#include <functional>
#include <map>
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
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
        m_box(box), m_root(std::make_unique<Node>(box)), m_getAABB(getAABB), m_equal(equal)
    { }

    void add(const T& value)
    {
        add(m_root.get(), 0, m_box, value);
    }

    void remove(const T& value)
    {
        remove(m_root.get(), m_box, value);
    }

    std::vector<T> query(const AABB& box) const
    {
        auto values = std::vector<T>();
        query(m_root.get(), m_box, box, values);
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
        return findAllIntersections(m_root.get());
    }

    AABB getAABB() const 
    {
        return m_box;
    }
    void updateLeafes() {
        updateLeafes(m_root.get());
    }
    void clear() {
        clear(m_root.get());

    }
    
private:
    static constexpr auto threshold = std::size_t(16);
    static constexpr auto max_depth = std::size_t(8);

    struct Node
    {
        std::array<std::unique_ptr<Node>, 4> children;
        std::vector<T> values;
        AABB box;

        Node(AABB b) : box(b) {}
    };

    AABB m_box;
    std::unique_ptr<Node> m_root;
    GetAABB m_getAABB;
    Equal m_equal;

    bool isLeaf(const Node* node) const
    {
        return !static_cast<bool>(node->children[0]);
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

    Node* add(Node* node, std::size_t depth, const AABB& box, const T& value)
    {
        assert(node != nullptr);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if (isLeaf(node))
        {
            if (depth >= max_depth || node->values.size() < threshold) {
                node->values.push_back(value);
                return node;
            } else {
                split(node, box);
            }
        }
        auto i = getQuadrant(box, m_getAABB(value));
        if (i != -1) {
            node->children[static_cast<std::size_t>(i)].get();
            return add(node->children[static_cast<std::size_t>(i)].get(), depth + 1, computeAABB(box, i), value);
        } else {
            node->values.push_back(value);
            return node;
        }
    }

    void split(Node* node, const AABB& box)
    {
        assert(node != nullptr);
        assert(isLeaf(node) && "Only leaves can be split");
        int quad = 0;
        for (auto& child : node->children)
            child = std::make_unique<Node>(computeAABB(node->box, quad++));
        auto newValues = std::vector<T>(); 
        for (const auto& value : node->values)
        {
            auto i = getQuadrant(box, m_getAABB(value));
            if (i != -1)
                node->children[static_cast<std::size_t>(i)]->values.push_back(value);
            else
                newValues.push_back(value);
        }
        node->values = std::move(newValues);
    }

    bool remove(Node* node, const AABB& box, const T& value)
    {
        assert(node != nullptr);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if (isLeaf(node))
        {
            removeValue(node, value);
            return true;
        }
        else
        {
            auto i = getQuadrant(box, m_getAABB(value));
            if (i != -1)
            {
                return remove(node->children[static_cast<std::size_t>(i)].get(), computeAABB(box, i), value);
            }
            else
                removeValue(node, value);
            return false;
        }
    }

    void removeValue(Node* node, const T& value)
    {
        auto it = std::find_if(std::begin(node->values), std::end(node->values),
            [this, &value](const auto& rhs){ return m_equal(value, rhs); });
        assert(it != std::end(node->values) && "Trying to remove a value that is not present in the node");
        *it = std::move(node->values.back());
        node->values.pop_back();
    }

    bool tryMerge(Node* node)
    {
        assert(node != nullptr);
        assert(!isLeaf(node) && "Only interior nodes can be merged");
        auto nbValues = node->values.size();
        for (const auto& child : node->children)
        {
            if (!isLeaf(child.get()))
                return false;
            nbValues += child->values.size();
        }
        if (nbValues <= threshold)
        {
            node->values.reserve(nbValues);
            for (const auto& child : node->children)
            {
                for (const auto& value : child->values) {
                    node->values.push_back(value);
                }
            }
            for (auto& child : node->children)
                child.reset();
            return true;
        }
        else
            return false;
    }
    void updateLeafes(Node* node) {
        if(!isLeaf(node) && !tryMerge(node))
            for(auto& child : node->children) {
                if(child)
                    updateLeafes(child.get());
            }
    }
    void clear(Node* node) {
        if(node == nullptr)
            return;
        node->values.clear();
        for(auto& child : node->children) {
            clear(child.get());
        }
    }

    void query(Node* node, const AABB& box, const AABB& queryAABB, std::vector<T>& values) const
    {
        assert(node != nullptr);
        if(!isOverlappingAABBAABB(queryAABB, box)) {
            return;
        }
        for (const auto& value : node->values) {
            if (isOverlappingAABBAABB(queryAABB, m_getAABB(value)))
                values.push_back(value);
        }
        if (!isLeaf(node)) {
            for (auto i = std::size_t(0); i < node->children.size(); ++i) {
                auto childAABB = computeAABB(box, static_cast<int>(i));
                if (isOverlappingAABBAABB(queryAABB, childAABB))
                    query(node->children[i].get(), childAABB, queryAABB, values);
            }
        }
    }

    void searchIntersecionsInNode(Node* node, int depth,
        std::vector<std::pair<T, T>>& intersections,
        std::array<Node*, max_depth>& parent_stack) const
    {
        if(!node)
            return;
        for (size_t i = 0; i < node->values.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                if (isOverlappingAABBAABB(m_getAABB(node->values[i]), m_getAABB(node->values[j])))
                    intersections.emplace_back(node->values[i], node->values[j]);
            }
        }
        for(int deep = 0; deep < depth; deep++) {
            auto parent = parent_stack[deep];
            assert(parent != nullptr);
            for (size_t i = 0; i < node->values.size(); ++i) {
                for (size_t j = 0; j < parent->values.size(); ++j) {
                    if (isOverlappingAABBAABB(m_getAABB(node->values[i]), m_getAABB(parent->values[j])))
                        intersections.emplace_back(node->values[i], parent->values[j]);
                }
            }
        }
    }
    std::vector<std::pair<T, T>> findAllIntersections(Node* root) const 
    {
        std::vector<std::pair<T, T>> intersections;

        std::stack<std::pair<Node*, int>> to_process;
        std::array<Node*, max_depth> parent_stack;
        to_process.push({root, 0});
        while(!to_process.empty()) {
            auto node_info = to_process.top();
            to_process.pop();
            if(node_info.first == nullptr)
                continue;;
            searchIntersecionsInNode(node_info.first, node_info.second, intersections, parent_stack);
            if(isLeaf(node_info.first))
                continue;
            parent_stack[node_info.second] = node_info.first;
            for (const auto& child : node_info.first->children)
                 to_process.push({child.get(), node_info.second + 1});
        }
        return intersections;
    }
};

}
#endif
