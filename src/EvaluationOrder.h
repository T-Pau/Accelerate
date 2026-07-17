#ifdef IN_XLR8_EVALUATION_ORDER_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_EVALUATION_ORDER_H
#ifndef HAD_XLR8_EVALUATION_ORDER_H
#define HAD_XLR8_EVALUATION_ORDER_H

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

#include <unordered_set>
#include <vector>

#include "Entity/Entity.h"

/**
 * @brief Class for determining the evaluation order of entities based on their dependencies.
 *
 * It uses Kahn's algorithm to compute a topological sort of the entities, ensuring that each entity is evaluated only after all its dependencies have been evaluated.
 */
class EvaluationOrder {
  public:
    /**
     * @brief Computes the evaluation order of the given starting entities.
     *
     * The dependencies of the starting entities are recursively explored to determine the order in which they should be evaluated.
     *
     * @param starting_entities The entities to start the evaluation from.
     * @return A vector of entities in the order they should be evaluated.
     */
    static std::vector<Entity*> order(const std::vector<Entity*>& starting_entities);

  private:
    /**
     * @brief Constructs an EvaluationOrder object with the given starting entities.
     *
     * @param starting_entities The entities to start the evaluation from.
     */
    EvaluationOrder(std::vector<Entity*> starting_entities);

    /**
     * @brief Computes the evaluation order of the entities.
     *
     * This method implements Kahn's algorithm to perform a topological sort of the entities based on their dependencies.
     */
    void compute_order();

  private:
    /// @brief This represents a node in the dependency graph of entities to be evaluated.
    class Node {
      public:
        /**
         * @brief Constructs a node for the given entity.
         * @param entity The entity corresponding to this node.
         */
        Node(Entity* entity) : entity(entity) {}

        /// @brief The entity corresponding to this node.
        Entity* entity;

        /// @brief The nodes that this node depends on.
        std::unordered_set<Node*> dependencies;

        /// @brief The nodes that depend on this node.
        std::unordered_set<Node*> dependents;

        /**
         * @brief Adds a dependency to this node.
         *
         * @param node The node that this node depends on.
         */
        void add_dependency(Node* node) {
            dependencies.insert(node);
            node->dependents.insert(this);
        }

        /**
         * @brief Checks if this node is ready to be evaluated.
         * @return True if this node has no dependencies, false otherwise.
         */
        [[nodiscard]] bool is_ready() const { return dependencies.empty(); }

        /**
         * @brief Removes a dependency from this node.
         * @param node The node to remove from the dependencies.
         */
        void remove_dependency(Node* node) { dependencies.erase(node); }
    };

    /**
     * @brief Get a node for an entity in the dependency graph.
     *
     * If the entity is already in the graph, its corresponding node is returned. Otherwise, a new node is created for the entity and added to the graph.
     *
     * This method also recursively adds nodes for all dependencies of the entity to the graph.
     *
     * @param entity The entity to add.
     * @return The node corresponding to the added entity.
     */
    [[nodiscard]] Node* node_for(Entity* entity);

    /**
     * @brief Removes a node from the dependencies of all its dependents.
     *
     * It also moves all dependents that are now ready to be evaluated into the ready_nodes set.
     *
     * @param node The node to remove from its dependents.
     */
    void remove_from_dependents(Node* node);

    /**
     * @brief Places a node in the evaluation order.
     *
     * This method adds the node's entity to the ordered_entities vector and removes the node from the dependency graph.
     *
     * @param node The node to place in the evaluation order.
     */
    void place_node(Node* node);

    /*
     * @brief The nodes corresponding to the entities to be evaluated.
     *
     * This maps each entity to its corresponding node in the dependency graph.
     * It is also the owner of the nodes.
     */
    std::unordered_map<Entity*, Node> nodes;

    /// @brief The nodes that have dependencies and are not ready to be evaluated.
    std::unordered_set<Node*> blocked_nodes;

    /// @brief The nodes that have no dependencies and are ready to be evaluated.
    std::unordered_set<Node*> ready_nodes;

    /**
     * @brief The entities that have been added to the evaluation order.
     *
     * These are stored as entities since their nodes are no longer needed and have been freed.
     */
    std::unordered_set<Entity*> sorted_entities;

    /// @brief The entities in the order they should be evaluated.
    std::vector<Entity*> ordered_entities;
};

#endif // HAD_XLR8_EVALUATION_ORDER_H
#undef IN_XLR8_EVALUATION_ORDER_H
