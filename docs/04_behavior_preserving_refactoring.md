# 04. Behavior-Preserving Refactoring / 동작 보존 리팩토링

## 문서 목적 / Document Purpose

이 문서는 구조 변경 중 어떤 결과를 보존하고, 어떤 내부 구조를 분리했는지 설명한다.
샘플은 실제 게임 규칙을 재현하지 않고, state snapshot과 event trace로 관찰 가능한 toy behavior를 비교한다.

This document explains which results are preserved and which internal responsibilities are separated during the refactoring.
The sample does not reproduce real game rules. It compares observable toy behavior through state snapshots and event traces.

## 보존 대상 / Preserved Behavior

보존 대상은 내부 구현이 아니라 외부에서 관찰되는 결과다.

Preserved behavior means externally observable results, not identical internal implementation.

- `CharacterStateSnapshot`
- `std::vector<CharacterEvent>`
- damage 적용 결과 / damage result
- death event 순서 / death event order
- effect expiration event 순서 / effect expiration event order
- passive event 순서 / passive event order
- projectile event 발생 여부와 count / projectile event presence and count

실제 마이그레이션에서는 log order, sound/effect trigger order, random call order, frame timing read order도 확인 대상이 될 수 있다.

In a real migration, log order, sound/effect trigger order, random call order, and frame timing read order may also need to be preserved.

## 분리 대상 / Structure Separated

분리 대상은 legacy class 안에 섞여 있던 책임이다.

The separated structure is the set of responsibilities that used to be mixed inside the legacy class.

| Legacy responsibility / 레거시 책임 | Refactored target / 분리 대상 |
| --- | --- |
| global array-style access / 전역 배열 스타일 접근 | `CharacterAccessor` |
| concrete object creation / 구체 객체 생성 | `CharacterFactory` |
| reusable lifetime slots / 재사용 가능한 lifetime slot | `CharacterSlotStore` |
| common lifecycle order / 공통 lifecycle 순서 | `Character` base class |
| type-specific behavior / 타입별 동작 | virtual hooks |
| calculation helpers / 계산 helper | `CharacterMath` |
| type-specific tuning values / 타입별 tuning 값 | `CharacterTuningTable` |
| skill id based values / skill id 기반 값 | `SkillDefinitionTable` |
| passive timing branches / passive 시점 분기 | `PassiveRegistry` |
| projectile branches / projectile 분기 | `ShotPattern` |
| effect timer state / effect timer 상태 | `TimedEffectList` |
| return-request handling / return-request 처리 | `ReturnRequestModule` |

## CharacterEquivalenceTests가 비교하는 것 / What CharacterEquivalenceTests Compares

`tests/CharacterEquivalenceTests.cpp`는 같은 toy input을 `LegacyCharacter`와 refactored `Character`에 적용한다.

`tests/CharacterEquivalenceTests.cpp` runs the same toy input against `LegacyCharacter` and the refactored `Character`.

```text
same input
  |
  +--> LegacyCharacter
  |      -> Snapshot()
  |      -> Events()
  |
  +--> Refactored Character
         -> Snapshot()
         -> Events()

assert snapshots are equal
assert event traces are equal
```

이 테스트는 구조가 달라져도 관찰 결과가 유지되는지 확인한다.
특히 `Update`, `ApplyDamage`, effect expiration, passive timing, death event 순서가 깨지지 않는지 확인한다.

This test checks whether observable results remain the same even when the internal structure changes.
It specifically watches `Update`, `ApplyDamage`, effect expiration, passive timing, and death event order.

## Snapshot과 Trace가 줄이는 리스크 / Snapshot and Trace Risk Reduction

Snapshot 비교는 최종 상태 차이를 잡는다.

Snapshot comparison catches final state differences.

- id
- kind
- health
- energy
- active effect count
- alive

Event trace 비교는 실행 순서 차이를 잡는다.

Event trace comparison catches execution order differences.

- `Initialized`
- `Passive:OneShot`
- `PreUpdate`
- `ProjectileSpawned`
- `ReturnRequestChecked`
- `Passive:PostPassive`
- `Updated`
- `Damaged`
- `Died`

상태만 비교하면 순서 문제가 빠질 수 있다.
trace를 같이 비교하면 refactoring 중 호출 순서가 바뀌는 문제를 더 빨리 발견할 수 있다.

State comparison alone can miss order changes.
Comparing the trace as well makes call-order regressions easier to find during refactoring.

## 호환 경계 / Compatibility Boundary

`CharacterAccessor`는 legacy array-style access를 바로 제거하지 않는다.
먼저 접근 계층을 둔다.

`CharacterAccessor` does not remove legacy array-style access immediately.
It first places an access boundary around it.

```text
legacy style
  gCharacters[index]

transition shape
  CharacterAccessor[index]

later shape
  explicit Character API
```

이 단계는 대규모 호출부를 한 번에 바꾸지 않기 위한 중간 구조다.

This is an intermediate shape that avoids changing a large number of call sites at once.
