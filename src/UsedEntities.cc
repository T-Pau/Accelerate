/*
UsedEntities.cc -- 

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

#include "UsedEntities.h"

#include <unordered_set>

void UsedEntities::insert(Entity* entity, bool optional) {
    auto it = entities.find(entity);

    if (it != entities.end()) {
        it->second = it->second && optional;
    }
    else {
        entities[entity] = optional;
    }
}

void UsedEntities::insert(const UsedEntities& other) {
    for (auto& [entity, optional]: other.entities) {
        insert(entity, optional);
    }
}


void UsedEntities::remove_optional() {
    auto optionl_entities = std::unordered_set<Entity*>{};

    for (auto& [entity, optional]: entities) {
        if (optional) {
            optionl_entities.insert(entity);
        }
    }

    for (const auto& entity: optionl_entities) {
        entities.erase(entity);
    }
}
