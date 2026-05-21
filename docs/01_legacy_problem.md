# 01. Legacy Problem

This document describes the problem shape represented by the toy legacy sample. It does not describe a specific commercial codebase.

> Korean note: 이 문서는 특정 회사 코드가 아니라, 오래된 C++ 게임 클라이언트에서 흔히 나타나는 구조적 문제를 일반화한 것입니다.

## Giant Character Class

The legacy sample puts lifecycle, damage, passive behavior, projectile behavior, effect timers, and return-request behavior into `LegacyCharacter`. This makes every change risky because a small modification can affect unrelated behavior.

The sample is intentionally larger than a clean example.
It keeps skill slots, status timers, projectile allocation, summon ownership, AI state, passive flags, and visual/sound hints in the same class.

> Korean note: 이 샘플은 좋은 예제가 아니라 문제를 보여주는 예제입니다. 하나의 Character 클래스에 상태와 분기가 계속 추가되는 모습을 보여줍니다.

## Global Array Access

Old call sites often assume that character objects can be reached through a global array-like registry. That is convenient, but it spreads lifetime and validity assumptions across the codebase.

## Type Branch Accumulation

The legacy sample uses `switch (CharacterKind)` to express type differences. This works for a small number of types, but it becomes fragile when every new type adds another exception.

## Mixed Responsibilities

The main issue is not that the code is old. The issue is that unrelated reasons to change are packed into the same class.

> Korean note: 리팩토링의 출발점은 못생긴 코드가 아니라 변경 이유가 한 곳에 너무 많이 모여 있는 구조입니다.

## Legacy Hotspot Map

| Hotspot | Legacy sample shape | Refactored boundary |
| --- | --- | --- |
| Global access | `gCharacters`, `LegacyAt`, `LegacyRawAt` | `CharacterAccessor` |
| Update order | AI, timers, passive, projectile, return-request in `Update()` | `Character` lifecycle |
| Skill data | cooldown, cost, power, and flags stored in Character | `SkillDefinitionTable`, `SkillExecutor` |
| Type-specific tuning | magic values near branch code | `CharacterTuningTable`, `CharacterMath` |
| Passive timing | before attack, after attack, damaged, dying, per-frame in one if-chain | `PassiveRegistry` |
| Projectile creation | straight, multi, directional, homing, sector functions in Character | `ShotPattern` |
| Effect timers | poison, stun, burn, shield, slow, silence, root, freeze, haste fields | `TimedEffectList` |
| Summon lifetime | owner, host, lifeTime, cleanup in Character | separate ownership/lifetime module candidate |
| Allocation policy | `new` and `delete` in gameplay functions | slot store or memory boundary |

The table does not map to a production source file.
It describes the synthetic problem shape used by this public sample.

## Why The Legacy Sample Is Verbose

The legacy code repeats similar logic on purpose.
For example, projectile creation functions allocate raw objects and fill position, direction, lifetime, and ownership fields in slightly different ways.
The repeated code makes the refactored `ShotPattern` boundary easier to compare.

The same rule is used for skill and timer state.
Cooldowns, costs, powers, passive flags, and status timers stay inside `LegacyCharacter` so the reader can see why `SkillDefinitionTable`, `SkillExecutor`, and `TimedEffectList` exist.
