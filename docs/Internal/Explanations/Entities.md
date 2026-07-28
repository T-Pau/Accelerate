# Entities

Entities represent the parts of a program.

**Entity** is the base class for all entities.

 **ScopeEntity** is for entities that introduce new names for their contained **Body** or **Expression**, either explicitly via assignment or implicitly via labels or arguments.
 
``` mermaid
---
  config:
    class:
      hideEmptyMembersBox: true
---
classDiagram
  Entity <|-- Constant
  Entity <|-- ScopeEntity
  ScopeEntity <|-- Function
  ScopeEntity <|-- Macro
  ScopeEntity <|-- Object
  ScopeEntity <|-- Output
class Entity <<Abstract>>
class ScopeEntity <<Abstract>>
```
