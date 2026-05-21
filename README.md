# Behavior-Preserving Refactoring of a Legacy C++ Game Character System

This repository contains a small C++17 sample that refactors a legacy-style game character system.
The code is anonymized and reconstructed for portfolio and educational use.

The sample compares two implementations of the same toy behavior:

- `src/legacy`: a centralized `LegacyCharacter` implementation.
- `src/refactored`: a modular implementation with access, creation, update, calculation, and timing responsibilities separated.

The sample does not include production source code, production data, internal names, or real game rules.

## Overview

The legacy version keeps most character-related behavior in one class.
It uses array-style global access, type checks, and local helper functions inside `LegacyCharacter`.

The refactored version keeps the externally observable toy behavior the same.
It introduces an access layer, a base character lifecycle, type-specific hooks, reusable object slots, stateless helper functions, passive handlers, shot patterns, and effect timer tracking.

The tests compare snapshots and event traces between the two implementations.

## Problem: Legacy Character System

The legacy sample represents a common structure in older game client code.
Character behavior is added to one central class over time.

In this sample, `LegacyCharacter` contains several responsibilities:

- character initialization and update
- damage and death handling
- type-based behavior branches
- passive timing branches
- projectile behavior
- effect timer state
- return-request behavior
- small utility calculations

This structure makes changes harder to isolate.
Adding a new character type requires edits inside the same methods that already handle existing types.
Testing one behavior also requires setting up unrelated character state.

## Refactoring Goals

The refactored sample changes the structure while keeping the toy behavior comparable.

- `CharacterAccessor` wraps array-like character access.
- `Character` owns the lifecycle order.
- Derived character classes handle type-specific hook behavior.
- `CharacterFactory` creates concrete character types.
- `CharacterSlotStore` manages reusable character slots.
- `CharacterMath` contains stateless calculations.
- `PassiveRegistry` executes passive handlers by timing.
- `ShotPattern` objects handle projectile variations.
- `TimedEffectList` stores and updates effect durations.
- `ReturnRequestModule` handles return-request events.

The implementation uses C++17 and the standard library only.
It avoids framework dependencies and complex template code.

## Architecture Before / After

```text
Before

LegacyRegistry[index]
  -> LegacyCharacter
       -> Initialize
       -> Update
       -> ApplyDamage
       -> switch by CharacterKind
       -> passive if/switch branches
       -> projectile branches
       -> effect timer fields
       -> local utility functions
```

```text
After

CharacterAccessor[index]
  -> Character
       -> fixed lifecycle order
       -> virtual hooks
       -> TimedEffectList
       -> PassiveRegistry
       -> ShotPattern
       -> ReturnRequestModule
       -> AiDispatcher

CharacterFactory
  -> concrete Character type

CharacterSlotStore
  -> reusable ownership slots
```

## Key Changes

### Global Access to CharacterAccessor

The legacy sample exposes characters through an array-style registry.
The refactored sample places `CharacterAccessor` between callers and stored character pointers.

`CharacterAccessor::operator[]` keeps array-like syntax for compatibility.
It also checks invalid indexes and empty slots at the access boundary.

### Giant Character Class to Character Hierarchy

The legacy sample uses one class for common behavior and type-specific behavior.
The refactored sample uses `Character` as the base class.

Concrete types such as `PlayerCharacter`, `MonsterCharacter`, `SummonCharacter`, and `EliteMonsterCharacter` override narrow hook points.
Common lifecycle order remains in the base class.

### Type Branches to Virtual Hooks

The legacy update path uses type checks inside `LegacyCharacter`.
The refactored update path calls virtual hooks from the base lifecycle.

New type-specific behavior can be added in a derived class without extending a large `switch` body in the main update method.

### Scattered Creation to Factory and Reusable Slots

`CharacterFactory` creates concrete character objects and attaches default collaborators.
`CharacterSlotStore` tracks acquired and released character slots.

This separates object creation from object reuse.
Tests can check acquire, release, capacity, and reuse behavior directly.

### Utility Methods to Stateless Helpers

Small calculations are moved to `CharacterMath`.
The helper functions do not store character state.

This keeps calculation tests independent from character lifecycle setup.

### Passive Branches to Handler Registry

The legacy sample executes passive behavior through timing branches.
The refactored sample stores passive handlers in `PassiveRegistry`.

Handlers are grouped by `PassiveTiming`:

- `OneShot`
- `Passive`
- `PostPassive`
- `Dying`

The execution timing is explicit in the lifecycle.

### Projectile Functions to Shot Strategies

Projectile variations are represented by `ShotPattern` implementations.
The sample includes straight, multi-shot, directional, sector, and homing patterns.

Each pattern emits generic toy events.
The code does not model real projectile rules.

### Effect Timers to TimedEffectList

The legacy sample stores effect timer state in `LegacyCharacter`.
The refactored sample moves timer storage and expiration events to `TimedEffectList`.

`Character` calls the tracker during update.
The tracker owns duration updates and expiration event emission.

### Allocation Policy to Memory Pool Example

`SimpleMemoryPool` is a small free-list example.
It keeps allocation policy outside gameplay objects.

The sample pool is not a production allocator.

## Behavior Preservation Strategy

The sample treats behavior preservation as an implementation constraint.

The refactored lifecycle keeps the order of the toy operations stable:

```text
Initialize
OneShot passive
PreUpdate event
type hook
effect update
Passive timing
shot pattern
return-request handling
PostPassive timing
PostUpdate hook
Updated event
damage
Dying passive
death event
```

The tests compare the externally visible results:

- state snapshot
- event trace
- accessor failure behavior
- reusable slot behavior
- virtual hook call order

For a real migration, the same approach would also check log order, effect trigger order, random call order, and frame timing reads.

## Testing Strategy

The repository uses simple `assert`-based tests.
No external test framework is required.

Test files:

- `CharacterEquivalenceTests.cpp`: compares legacy and refactored snapshots and event traces.
- `CharacterAccessorTests.cpp`: checks valid access, invalid index access, and empty slot access.
- `CharacterSlotStoreTests.cpp`: checks acquire, release, capacity, and reuse behavior.
- `VirtualHookOrderTests.cpp`: checks lifecycle hook order.

Build and run:

```bash
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Repository Layout

```text
docs/
  01_legacy_problem.md
  02_refactoring_strategy.md
  03_architecture_overview.md
  04_behavior_preserving_refactoring.md
  05_risk_and_limitations.md
  06_public_release_checklist.md
src/
  legacy/
  refactored/
tests/
```

## Limitations

This repository is a toy project.

It does not include:

- production source code
- production data
- networking code
- scripting integration
- asset loading
- content table parsing
- real combat, skill, or AI rules
- production memory management

The sample code uses small numeric values and generic event names.

## Security and Anonymization Notice

This repository is reconstructed from general refactoring experience.
It does not contain company source code or internal project material.

The sample intentionally excludes:

- company names
- game titles
- internal project names
- real class, function, variable, and file names
- character, skill, named boss, item, region, server, packet, and table names
- production algorithms, numeric tuning, data formats, and operational rules
- internal comments, paths, log messages, screenshots, and source history
