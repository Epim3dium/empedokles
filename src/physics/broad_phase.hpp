#ifndef EMP_BROADPHASE_HPP
#define EMP_BROADPHASE_HPP
#include <set>
#include <stack>
#include <vector>
#include "core/entity.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
#include "templates/free_list.hpp"
namespace emp {
struct CollidingPair {
    Entity e1;
    Entity e2;
    size_t shape_index1;
    size_t shape_index2;
};
class SweepBroadPhase{
public:
    std::vector<CollidingPair> findPotentialPairs(std::set<Entity>::iterator begin, std::set<Entity>::iterator end);
};
class Quadtree {
    struct Node  {
        int32_t first_child = -1;
        int32_t count = 0;
    };

    struct Element {
        Entity data;
        AABB aabb;
    };
    struct LinkedElement {
        int next = -1;
        int element;
    };
    FreeList<Element> elements;
    FreeList<LinkedElement> linked_elements;
    FreeList<Node> nodes;
    int free_node = -1;

    AABB root_rect;

private:
    AABB Rect0(AABB parent) const {
        return AABB::CreateMinMax(parent.bl(), parent.center());
    };
    AABB Rect1(AABB parent) const {
        return AABB::CreateMinMax({parent.center().x, parent.bl().y}, {parent.br().x, parent.center().y});
    };
    AABB Rect2(AABB parent) const {
        return AABB::CreateMinMax(parent.center(), parent.tr());
    };
    AABB Rect3(AABB parent) const {
        return AABB::CreateMinMax({parent.bl().x, parent.center().y}, {parent.center().x, parent.tr().y});
    };
    struct NodeData {
        AABB area;
        int index;
        int depth_left = 8;
    };
    int divide(int node_index, AABB rect) {
        assert(nodes[node_index].count != -1);
        auto& node = nodes[node_index];
        int first_linked_element = nodes[node_index].first_child;
        node.first_child = std::min({nodes.insert(Node()), nodes.insert(Node()), nodes.insert(Node()), nodes.insert(Node())});
        node.count = -1;
        int to_readd = first_linked_element;
        while(to_readd != -1) {
            int cur_linked_index = to_readd;
            to_readd = linked_elements[cur_linked_index].next;

            const auto& element = elements[linked_elements[cur_linked_index].element];
            auto rect0 = Rect0(rect);
            auto rect1 = Rect0(rect);
            auto rect2 = Rect0(rect);
            auto rect3 = Rect0(rect);
            int i = 0;
            for(auto r : {rect0, rect1, rect2, rect3}) {
                if(AABBcontainsAABB(r, element.aabb)) {
                    m_insertIntoNode(node.first_child + i, cur_linked_index);
                }
                i++;
            }
        }
    }
    std::vector<NodeData> m_findLeaves(int root = 0) {
        return m_findLeaves(root_rect, root);
    }
    std::vector<NodeData> m_findLeaves(AABB area, int root = 0) {
        std::vector<NodeData> result;
        std::stack<NodeData> to_process;
        to_process.push({area, root, max_depth});
        while(to_process.size() > 0) {
            auto node = to_process.top();
            to_process.pop();
            if(node.depth_left < 0) {
                continue;
            }
            //is a leaf, push back
            if(nodes[node.index].count != -1) {
                result.push_back(node);
            }else {
                //push intersecting children
                auto rect = Rect0(area);
                if(isOverlappingAABBAABB(rect, area)) {
                    to_process.push({ rect, nodes[node.index].first_child+0, node.depth_left - 1} );
                }
                rect = Rect1(area);
                if(isOverlappingAABBAABB(rect, area)) {
                    to_process.push({ rect, nodes[node.index].first_child+1, node.depth_left - 1} );
                }
                rect = Rect2(area);
                if(isOverlappingAABBAABB(rect, area)) {
                    to_process.push({ rect, nodes[node.index].first_child+2, node.depth_left - 1} );
                }
                rect = Rect3(area);
                if(isOverlappingAABBAABB(rect, area)) {
                    to_process.push({ rect, nodes[node.index].first_child+3, node.depth_left - 1} );
                }
            }
        }
        return result;
    }
    int findBestFitFor(AABB object, int root = 0) {
        std::stack<NodeData> to_process;
        to_process.push({object, root, max_depth});
        while(to_process.size() > 0) {
            auto node = to_process.top();
            to_process.pop();
            if(node.depth_left < 0) {
                continue;
            }
            auto rect0 = Rect0(node.area);
            auto rect1 = Rect0(node.area);
            auto rect2 = Rect0(node.area);
            auto rect3 = Rect0(node.area);
            int i = 0;
            for(auto r : {rect0, rect1, rect2, rect3}) {
                if(AABBcontainsAABB(r, object) && node.depth_left == ) {
                    to_process.push({r, nodes[node.index].first_child + i, node.depth_left - 1});
                    break;
                }
                i++;
            }
            
        }

    }
    void cleanup() {
        // Only process the root if it's not a leaf.
        std::vector<int> to_process;
        if (nodes[0].count == -1)
            to_process.push_back(0);

        while (to_process.size() > 0)
        {
            const int node_index = to_process.back();
            to_process.pop_back();
            auto& node = nodes[node_index];

            // Loop through the children.
            int num_empty_leaves = 0;
            for (int j=0; j < 4; ++j)
            {
                const int child_index = node.first_child + j;
                const auto& child = nodes[child_index];

                // Increment empty leaf count if the child is an empty 
                // leaf. Otherwise if the child is a branch, add it to
                // the stack to be processed in the next iteration.
                if (child.count == 0)
                    ++num_empty_leaves;
                else if (child.count == -1)
                    to_process.push_back(child_index);
            }

            // If all the children were empty leaves, remove them and 
            // make this node the new empty leaf.
            if (num_empty_leaves == 4)
            {
                // Push all 4 children to the free list.
                nodes[node.first_child].first_child = free_node;
                free_node = node.first_child;

                // Make this node the new empty leaf.
                node.first_child = -1;
                node.count = 0;
            }
        }
    }
    int m_insertElement(Entity data, AABB aabb) {
        auto index = elements.insert({data, aabb});
        auto linked_index = linked_elements.insert({-1, index});
        return linked_index;
    }
    void m_insertIntoNode(int node_index, int linked_element_index) {
        auto& node = nodes[node_index];
        linked_elements[linked_element_index].next = node.first_child;
        node.first_child = linked_element_index;
        node.count++;
    }
public:
    int max_depth = 8;
    Quadtree(size_t max_entities_count, AABB max_quadtree_size);
    void add(Entity data, AABB aabb) {
        int node_index = m_findNode(aabb);
        int linked_element = m_insertElement(data, aabb);
        m_insertIntoNode(node_index, linked_element);
    }
    void remove(Entity data, AABB aabb);
    void query(vec2f point);
    void query(AABB area);
};
};
#endif
