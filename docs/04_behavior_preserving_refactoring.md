# 04. Behavior-Preserving Refactoring

이 문서는 구조 변경 중 어떤 결과를 보존하고, 어떤 내부 구조를 분리했는지 설명한다.
샘플은 실제 게임 규칙을 재현하지 않고, state snapshot과 event trace로 관찰 가능한 toy behavior를 비교한다.

## Preserved Behavior

보존 대상은 내부 구현이 아니라 외부에서 관찰되는 결과다.

- `CharacterStateSnapshot`
- `std::vector<CharacterEvent>`
- damage 적용 결과
- death event 순서
- effect expiration event 순서
- passive event 순서
- projectile event 발생 여부와 count

실제 마이그레이션에서는 여기에 log order, sound/effect trigger order, random call order, frame timing read order가 추가될 수 있다.

## Structure Separated

분리 대상은 legacy class 안에 섞여 있던 책임이다.

| Legacy responsibility | Refactored target |
| --- | --- |
| global array-style access | `CharacterAccessor` |
| concrete object creation | `CharacterFactory` |
| reusable lifetime slots | `CharacterSlotStore` |
| common lifecycle order | `Character` base class |
| type-specific behavior | virtual hooks |
| calculation helpers | `CharacterMath` |
| type-specific tuning values | `CharacterTuningTable` |
| skill id based values | `SkillDefinitionTable` |
| passive timing branches | `PassiveRegistry` |
| projectile branches | `ShotPattern` |
| effect timer state | `TimedEffectList` |
| return-request handling | `ReturnRequestModule` |

## What CharacterEquivalenceTests Compares

`tests/CharacterEquivalenceTests.cpp` runs the same toy input against `LegacyCharacter` and refactored `Character`.

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

## Snapshot and Trace Risk Reduction

Snapshot 비교는 최종 상태 차이를 잡는다.

- id
- kind
- health
- energy
- active effect count
- alive

Event trace 비교는 실행 순서 차이를 잡는다.

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

## Compatibility Boundary

`CharacterAccessor`는 legacy array-style access를 바로 제거하지 않는다.
먼저 접근 계층을 둔다.

```text
legacy style
  gCharacters[index]

transition shape
  CharacterAccessor[index]

later shape
  explicit Character API
```

이 단계는 대규모 호출부를 한 번에 바꾸지 않기 위한 중간 구조다.
