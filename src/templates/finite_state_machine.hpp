#ifndef EMP_FINITE_STATE_MACHINE
#define EMP_FINITE_STATE_MACHINE

#include <string>
#include <functional>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <memory>

namespace emp {
template <class Identifier_t, class ...TriggerFuncInput_t>
class FiniteStateMachine {
    typedef std::function<bool(TriggerFuncInput_t...)> TriggerFunc_t;

    struct Edge;
    struct Node {
        Identifier_t name;
        std::vector<Edge> outgoing_edges;

        Node() {}
        Node(Identifier_t id) : name(id) {}
    };
    struct Edge {
        Identifier_t from;
        Identifier_t to;
        TriggerFunc_t trigger;
        Edge(Identifier_t from_, Identifier_t to_, TriggerFunc_t trigger_) : from(from_), to(to_), trigger(trigger_) {}
    };

    std::unordered_map<Identifier_t, Node > m_nodes;
    Identifier_t m_current_node;
public:
    class Builder {
    protected:
        std::unordered_map<Identifier_t, Node > nodes;
        Identifier_t entry_point;
    public:
        Builder(Identifier_t entry_point_node) {
            addNode(entry_point_node);
            entry_point = entry_point_node;
        }
        void addNode(Identifier_t name) {
            assert(!nodes.contains(name) && "cannot have more than 1 node of the same name in state machine");
            nodes[name] = Node(name);
        }
        void addEdge(Identifier_t from, Identifier_t to, TriggerFunc_t trigger) {
            assert(nodes.contains(from) && "tried to create edge without start");
            assert(nodes.contains(to) && "tried to create edge without end");

            Identifier_t from_ptr = nodes.at(from).name;
            Identifier_t to_ptr = nodes.at(to).name;
            nodes.at(from).outgoing_edges.push_back(Edge(from_ptr, to_ptr, trigger));
        }
        friend FiniteStateMachine;
    };

    const Identifier_t state() const {
        assert(m_nodes.contains(m_current_node) && "empty FSM impossible");
        return m_nodes.at(m_current_node).name;
    }

    Identifier_t eval(TriggerFuncInput_t... input) {
        Identifier_t prev_current;

        constexpr int max_iter_count = 100;
        int iter_count = 0;
        for(; iter_count < max_iter_count && m_current_node != prev_current; iter_count++) {
            prev_current = m_current_node;
            assert(m_nodes.contains(m_current_node) && "empty FSM impossible");
            auto& node = m_nodes.at(m_current_node);
            for(const auto& edge : node.outgoing_edges) {
                assert(edge.from == m_current_node && "this must be true");
                if(edge.trigger(input...) == true) {
                    m_current_node = edge.to;
                    break;
                }
            }
        }
        return m_current_node;
    }
    FiniteStateMachine() = delete;
    FiniteStateMachine(const Builder& builder) : m_nodes(builder.nodes), m_current_node(builder.entry_point) {
    }
};
};
#endif //EMP_FINITE_STATE_MACHINE
