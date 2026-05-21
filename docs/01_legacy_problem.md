# 01. Legacy Problem / 레거시 문제 구조

## 문서 목적 / Document Purpose

이 문서는 공개용 toy legacy sample이 어떤 문제 구조를 표현하는지 설명한다.
특정 상용 코드베이스를 설명하지 않는다.

This document explains the problem shape represented by the public toy legacy sample.
It does not describe a specific commercial codebase.

## 거대한 Character 클래스 / Giant Character Class

레거시 샘플은 lifecycle, damage, passive, projectile, effect timer, return-request 처리를 `LegacyCharacter` 안에 함께 둔다.
작은 변경도 관련 없는 동작에 영향을 줄 수 있는 구조를 보여준다.

The legacy sample keeps lifecycle, damage, passive behavior, projectile behavior, effect timers, and return-request handling inside `LegacyCharacter`.
This shape makes small changes likely to affect unrelated behavior.

이 샘플은 좋은 예제가 아니라 문제를 보여주는 예제다.
skill slot, status timer, projectile allocation, summon ownership, AI state, passive flag, visual/sound hint가 같은 클래스에 누적되는 모습을 표현한다.

The sample is intentionally larger than a clean example.
It keeps skill slots, status timers, projectile allocation, summon ownership, AI state, passive flags, and visual/sound hints in the same class.

## 전역 배열 접근 / Global Array Access

오래된 호출부는 Character 객체를 전역 배열 형태로 접근한다고 가정하기 쉽다.
이 방식은 호출은 간단하지만 lifetime, index validity, empty slot 처리를 코드 전체로 퍼뜨린다.

Old call sites often assume that character objects can be reached through a global array-like registry.
This keeps call sites short, but it spreads lifetime, index validity, and empty slot assumptions across the codebase.

## 타입 분기 누적 / Type Branch Accumulation

레거시 샘플은 `switch`와 `if-chain`으로 타입별 차이를 처리한다.
타입이 적을 때는 동작하지만, 새 타입과 예외가 늘수록 기존 함수 본문을 계속 수정해야 한다.

The legacy sample uses `switch` and `if-chain` logic to express type differences.
This works for a small number of types, but each new type or exception requires changes inside existing functions.

## 책임 혼재 / Mixed Responsibilities

문제는 코드가 오래되었다는 사실이 아니다.
서로 다른 변경 이유가 같은 클래스와 같은 함수 안에 모여 있다는 점이다.

The issue is not that the code is old.
The issue is that unrelated reasons to change are packed into the same class and the same functions.

## Legacy Hotspot Map / 레거시 Hotspot Map

| Hotspot | Legacy sample shape | Refactored boundary |
| --- | --- | --- |
| Global access / 전역 접근 | `gCharacters`, `LegacyAt`, `LegacyRawAt` | `CharacterAccessor` |
| Update order / 업데이트 순서 | AI, timers, passive, projectile, return-request in `Update()` | `Character` lifecycle |
| Skill data / 스킬 데이터 | cooldown, cost, power, and flags stored in Character | `SkillDefinitionTable`, `SkillExecutor` |
| Type tuning / 타입별 tuning | magic values near branch code | `CharacterTuningTable`, `CharacterMath` |
| Passive timing / passive 시점 | before attack, after attack, damaged, dying, per-frame in one if-chain | `PassiveRegistry` |
| Projectile creation / 투사체 생성 | straight, multi, directional, homing, sector functions in Character | `ShotPattern` |
| Effect timers / 상태 timer | poison, stun, burn, shield, slow, silence, root, freeze, haste fields | `TimedEffectList` |
| Summon lifetime / 소환 lifetime | owner, host, lifeTime, cleanup in Character | separate ownership/lifetime module candidate |
| Allocation policy / 할당 정책 | `new` and `delete` in gameplay functions | slot store or memory boundary |

이 표는 production source file과 매핑되지 않는다.
공개 샘플에서 사용한 synthetic problem shape를 설명한다.

This table does not map to a production source file.
It describes the synthetic problem shape used by this public sample.

## 레거시 샘플이 긴 이유 / Why The Legacy Sample Is Verbose

레거시 코드는 비슷한 로직을 의도적으로 반복한다.
예를 들어 projectile 생성 함수들은 raw object를 할당하고 position, direction, lifetime, ownership field를 조금씩 다른 방식으로 채운다.
이 반복은 refactored 쪽 `ShotPattern` 경계와 비교하기 쉽게 만든다.

The legacy code repeats similar logic on purpose.
For example, projectile creation functions allocate raw objects and fill position, direction, lifetime, and ownership fields in slightly different ways.
This repetition makes the refactored `ShotPattern` boundary easier to compare.

skill과 timer 상태도 같은 기준으로 작성했다.
cooldown, cost, power, passive flag, status timer를 `LegacyCharacter` 안에 두어 `SkillDefinitionTable`, `SkillExecutor`, `TimedEffectList`가 필요한 이유를 보여준다.

The same rule is used for skill and timer state.
Cooldowns, costs, powers, passive flags, and status timers stay inside `LegacyCharacter` so the reader can see why `SkillDefinitionTable`, `SkillExecutor`, and `TimedEffectList` exist.
