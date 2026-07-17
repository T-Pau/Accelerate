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
            // TODO: find entities involved in the cycle and report them, and break the cycle to continue with the rest of the entities
            throw Exception("circular dependency detected");
        }
    }
}

EvaluationOrder::Node* EvaluationOrder::node_for(Entity* entity) {
    auto [it, inserted] = nodes.try_emplace(entity, entity);
    auto node = &it->second;

    if (!inserted) {
        return node;
    }

    for (auto* dependency : entity->referenced_entities) {
        if (dependency == entity) {
            DiagnosticOutput::global.error("entity {} depends on itself", entity->name);
            continue;
        }
        entity->resolve();
        if (dependency->as<Object>()) {
            // Entities don't depend on the evaluation of objects, just their address. But we still need to add it to the graph to make sure it is processed correctly.
            (void)node_for(dependency);
        }
        else {
            auto dependency_node = node_for(dependency);
            node->add_dependency(dependency_node);
        }
    }

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
