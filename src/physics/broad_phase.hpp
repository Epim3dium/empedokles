#ifndef EMP_BROADPHASE_HPP
#define EMP_BROADPHASE_HPP
#include <set>
#include <stack>
#include <algorithm>
#include <vector>
#include "core/entity.hpp"
#include "debug/log.hpp"
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
struct LooseTightDoubleGrid {
    struct LooseCell {
        int head;
        AABB area;
    };
    struct LooseCellNode {
        // Points to the next loose cell node in the tight cell.
        int next = -1;

        // Stores an index to the loose cell.
        int cell_idx;
    };
    struct DataNode {
        int next = -1;
        AABB area;
        Entity entity;
    };

    struct TightCell {
        int head = -1;
    };
    vec2f bottom_left_corner;
    vec2f inv_cell_size;
    vec2f cell_size;
    int num_cols;
    int num_rows;
    std::vector<LooseCell> loose_cells;

    //points to linked_loose_cells
    std::vector<TightCell> tight_cells;
    FreeList<LooseCellNode> linked_loose_cells;
    FreeList<DataNode> linked_data;
    std::map<Entity, int> entities_in_loose_cells;

    int cellIndex(vec2f position) {
        int cell_x = std::clamp<int>(floor((position.x - bottom_left_corner.x) * inv_cell_size.x), 0, num_cols-1);
        int cell_y = std::clamp<int>(floor((position.y - bottom_left_corner.y) * inv_cell_size.y), 0, num_rows-1);
        int cell_idx = cell_y*num_rows + cell_x;
        return cell_idx;
    }
    AABB tightAABB(int index) {
        vec2f min = {(index % num_rows) * cell_size.x, (index / num_cols) * cell_size.y};
        return AABB::CreateMinMax(min + bottom_left_corner, min + cell_size + bottom_left_corner);
    }
    bool tightContains(TightCell tight_cell, int cell_idx_loose) {
        int next = tight_cell.head;
        while(next != -1) {
            if(linked_loose_cells[next].cell_idx == cell_idx_loose) {
                return true;
            }
            next = linked_loose_cells[next].next;
        }
        return false;
    }
    std::vector<Entity> query(AABB queried_area) {
        bool isContained[MAX_ENTITIES]{0};
        std::vector<Entity> result;
        int minx = std::clamp<int>(floor((queried_area.min.x - bottom_left_corner.x) * inv_cell_size.x ), 0, num_cols-1);
        int maxx = std::clamp<int>(floor((queried_area.max.x - bottom_left_corner.x) * inv_cell_size.x ), 0, num_cols-1);
        int miny = std::clamp<int>(floor((queried_area.min.y - bottom_left_corner.y) * inv_cell_size.y ), 0, num_rows-1);
        int maxy = std::clamp<int>(floor((queried_area.max.y - bottom_left_corner.y) * inv_cell_size.y ), 0, num_rows-1);

        for(int y = miny; y <= maxy; y++) {
            int trow = y * num_cols;
            for(int x = minx; x <= maxx; x++) {
                int index = trow + x;
                auto& tight_cell = tight_cells[index];
                int current_loose_node = tight_cell.head;
                EMP_LOG_DEBUG << "analyzing: " << index;
                while(current_loose_node != -1) {
                    EMP_LOG_DEBUG << "in loose node: " << current_loose_node;
                    int loose_cell_idx = linked_loose_cells[current_loose_node].cell_idx;
                    int current_element = loose_cells[loose_cell_idx].head;
                    while(current_element != -1) {
                        if(isContained[linked_data[current_element].entity]) {
                            current_element = linked_data[current_element].next;
                            continue;
                        }

                        if(isOverlappingAABBAABB(linked_data[current_element].area, queried_area)) {
                            result.push_back(linked_data[current_element].entity);
                            isContained[result.back()] = true;
                        }
                        current_element = linked_data[current_element].next;
                    }
                    current_loose_node = linked_loose_cells[current_loose_node].next;
                }
            }
        }
        return result;
    }
    void insert(AABB object, Entity entity) {
        auto object_index = linked_data.insert(DataNode{-1, object, entity});

        auto loose_cell_index = cellIndex(object.center());
        auto& loose_cell = loose_cells[loose_cell_index];
        loose_cell.area.expandToContain(object.min);
        loose_cell.area.expandToContain(object.max);

        linked_data[object_index].next = loose_cell.head;
        loose_cell.head = object_index;
        entities_in_loose_cells[entity] = loose_cell_index;

        // linked_loose_cells.insert({tight_cells[cell_index].head, cell_index});

        int minx = std::clamp<int>(floor((object.min.x - bottom_left_corner.x) * inv_cell_size.x ), 0, num_cols-1);
        int maxx = std::clamp<int>(floor((object.max.x - bottom_left_corner.x) * inv_cell_size.x ), 0, num_cols-1);
        int miny = std::clamp<int>(floor((object.min.y - bottom_left_corner.y) * inv_cell_size.y ), 0, num_rows-1);
        int maxy = std::clamp<int>(floor((object.max.y - bottom_left_corner.y) * inv_cell_size.y ), 0, num_rows-1);

        EMP_LOG_DEBUG << loose_cell.area.min.x << " " << loose_cell.area.min.y << "\t" << loose_cell.area.max.x << " " << loose_cell.area.max.y;
        for(int y = miny; y <= maxy; y++) {
            int trow = y * num_cols;
            for(int x = minx; x <= maxx; x++) {
                int index = trow + x;
                auto& tight_cell = tight_cells[index];
                auto area = tightAABB(index);
                EMP_LOG_DEBUG <<"\t" << area.min.x << " " << area.min.y << "\t" << area.max.x << " " << area.max.y;
                if(isOverlappingAABBAABB(loose_cell.area, tightAABB(index))) {
                    tight_cell.head = linked_loose_cells.insert({tight_cell.head, loose_cell_index});
                }
            }
        }
    }
    void erase(Entity entity) {
        if(!entities_in_loose_cells.contains(entity)) {
            return;
        }
        int loose_cell_idx = entities_in_loose_cells.at(entity);
        entities_in_loose_cells.erase(entity);
        int current_idx = loose_cells[loose_cell_idx].head;
        int* prev_link = &loose_cells[loose_cell_idx].head;
        while(current_idx != -1) {
            if(linked_data[current_idx].entity == entity) {
                *prev_link = linked_data[current_idx].next;
                linked_data.erase(current_idx);
            }else {
                prev_link = &linked_data[current_idx].next;
                current_idx = linked_data[current_idx].next;
            }
        }
    }
    void update(AABB object, Entity entity) {
        erase(entity);
        insert(object, entity);
    }
    // vec2f bottom_left_corner;
    // vec2f inv_cell_size;
    // vec2f cell_size;
    // int num_cols;
    // int num_rows;
    // std::vector<LooseCell> loose_cells;
    //
    // //points to linked_loose_cells
    // std::vector<TightCell> tight_cells;
    // FreeList<LooseCellNode> linked_loose_cells;
    // FreeList<DataNode> linked_data;
    // std::map<Entity, int> entities_in_loose_cells;

    LooseTightDoubleGrid(AABB total_coverage, vec2f cell_size_)
        : cell_size(cell_size_), inv_cell_size(1.f / cell_size_.x, 1.f / cell_size_.y),
          bottom_left_corner(total_coverage.min), num_rows(total_coverage.size().y / cell_size.y + 1),
          num_cols(total_coverage.size().x / cell_size.x + 1), loose_cells(num_rows * num_cols, {-1, AABB::Expandable()}),
          tight_cells(num_rows * num_cols) {}
};
};
#endif
