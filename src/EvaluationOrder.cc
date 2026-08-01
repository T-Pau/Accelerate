/*
Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "EvaluationOrder.h"

#include <tpau-cpp-kernal/DiagnosticOutput.h>
#include <tpau-cpp-kernal/Exception.h>
#include <tpau-cpp-kernal/Util.h>

#include "Entity/Object.h"

using namespace tpau::cpp_kernal;

std::vector<Entity*> EvaluationOrder::order(const std::vector<Entity*>& starting_entities) {
    EvaluationOrder evaluation_order(starting_entities);
    evaluation_order.compute_order();
    return std::move(evaluation_order.ordered_entities);
}

EvaluationOrder::EvaluationOrder(std::vector<Entity*> starting_entities) {
    for (auto* entity : starting_entities) {
        (void)node_for(entity);
    }
}

void EvaluationOrder::compute_order() {
    while (!ready_nodes.empty() || !blocked_nodes.empty()) {
        while (!ready_nodes.empty()) {
            auto* node = *ready_nodes.begin();
            ready_nodes.erase(ready_nodes.begin());

            remove_from_dependents(node);
            place_node(node);
        }

        if (!blocked_nodes.empty()) {
            break_cycle();
        }
    }
}

EvaluationOrder::Node* EvaluationOrder::node_for(Entity* entity) {
    auto [it, inserted] = nodes.try_emplace(entity, entity);
    auto node = &it->second;

    if (!inserted) {
        return node;
    }

    entity->resolve();

#ifdef TRACE_TRANSLATION
    std::vector<Symbol> dependency_names;
#endif

    for (auto* dependency : entity->referenced_entities) {
        if (dependency == entity) {
            // TODO: mark entity as invalid
            DiagnosticOutput::global.error(entity->location, "circular definition: {} -> {}", entity->name, entity->name);
            continue;
        }
        if (dependency->as<Object>()) {
            // Entities don't depend on the evaluation of objects, just their address. But we still need to add it to the graph to make sure it is processed correctly.
            (void)node_for(dependency);
        }
        else {
#ifdef TRACE_TRANSLATION
            dependency_names.push_back(dependency->name);
#endif
            auto dependency_node = node_for(dependency);
            node->add_dependency(dependency_node);
        }
    }

#ifdef TRACE_TRANSLATION
    TRACE_INSTANCE(entity, "ordering", "{} depends on: {}", entity->name, join(dependency_names, ", "));
#endif

    if (node->is_ready()) {
        ready_nodes.insert(node);
    }
    else {
        blocked_nodes.insert(node);
    }
    return node;
}

void EvaluationOrder::place_node(EvaluationOrder::Node* node) {
    if (!node->is_ready()) {
        throw Exception("internal error: entity {} is ordered before its dependencies", node->entity->name);
    }

    ordered_entities.push_back(node->entity);

    nodes.erase(node->entity);
}

void EvaluationOrder::remove_from_dependents(Node* node) {
    for (auto* dependent : node->dependents) {
        dependent->remove_dependency(node);
        if (dependent->is_ready()) {
            blocked_nodes.erase(dependent);
            ready_nodes.insert(dependent);
        }
    }
}

void EvaluationOrder::break_cycle() {
    while (ready_nodes.empty()) {
        auto cycle = find_cycle();
        auto first = cycle.front();
        auto second = cycle.size() > 1 ? cycle[1] : cycle.front();
        auto names = std::vector<Symbol>();
        for (auto* node : cycle) {
            // TODO: mark entity as invalid
            names.push_back(node->entity->name);
        }
        names.push_back(first->entity->name);
        DiagnosticOutput::global.error(first->entity->location, "circular definition: {}", join(names, " -> "));
        first->remove_dependency(second);
        if (first->is_ready()) {
            blocked_nodes.erase(first);
            ready_nodes.insert(first);
        }
    }
}

std::vector<EvaluationOrder::Node*> EvaluationOrder::find_cycle() {
    auto stack = std::vector<TraversalPosition>();
    auto visited = std::unordered_set<Node*>();

    stack.emplace_back(*blocked_nodes.begin());

    while (!stack.empty()) {
        auto next = stack.back().next();
        if (next) {
            if (visited.contains(next)) {
                auto start = std::find_if(stack.begin(), stack.end(), [next](const TraversalPosition& pos) { return pos.node == next; });
                if (start == stack.end()) {
                    throw Exception("internal error: cycle detection failed");
                }
                auto cycle = std::vector<Node*>();
                for (auto it = start; it != stack.end(); ++it) {
                    cycle.push_back(it->node);
                }
                return cycle;
            }
            else {
                visited.insert(next);
                stack.emplace_back(next);
            }
        }
        else {
            stack.pop_back();
        }
    }
    throw Exception("internal error: no cycle found starting from node for entity {}", (*blocked_nodes.begin())->entity->name);
}
