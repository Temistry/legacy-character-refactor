# 03. Architecture Overview

이 문서는 현재 저장소에 실제로 들어 있는 모듈을 기준으로 구조를 설명한다.
`legacy/`는 문제를 보여주는 synthetic example이고, `refactored/`는 책임을 어떤 경계로 나눌 수 있는지 보여주는 샘플이다.

## Legacy View

```text
gCharacters[index] / LegacyRegistry[index]
  -> LegacyCharacter
       -> Initialize / Update / ApplyDamage / Die
       -> Move / Attack / ProcessAI
       -> CastSkill / UpdateSkillTimers
       -> ApplyPassiveEffects
       -> CreateStraightProjectile / CreateMultiProjectile
       -> SummonUnit / RecallUnit / CleanupSummon
       -> poisonTimer / stunTimer / burnTimer / shieldTimer
       -> skillCooldownA / passiveFlagA / passiveCounterB
       -> local utility calculations
       -> scattered new/delete
```

이 구조에서는 접근, 생성, 실행 순서, 타입별 예외, 상태 timer, 계산 로직이 `LegacyCharacter` 안에 함께 들어간다.
새 기능을 넣을 때 기존 함수 본문에 조건문과 상태 멤버가 계속 추가되는 형태를 만든다.

## Refactored View

```text
CharacterAccessor[index]
  -> Character
       -> Initialize
       -> Update
          -> OnPreUpdate hook
          -> AiDispatcher
          -> TimedEffectList
          -> PassiveRegistry
          -> ShotPattern
          -> ReturnRequestModule
          -> OnPostUpdate hook
       -> ApplyDamage
       -> Die

CharacterFactory
  -> concrete Character type
  -> default PassiveRegistry
  -> default ShotPattern

CharacterSlotStore
  -> acquired/released object slots
```

`Character`는 생명주기 순서를 잡고, 세부 책임은 collaborator에 넘긴다.
타입별 차이는 파생 클래스 hook 또는 독립 모듈에서 처리한다.

## Module Responsibility Map

| Module | File | Responsibility |
| --- | --- | --- |
| `CharacterAccessor` | `src/refactored/CharacterAccessor.*` | 배열 스타일 접근을 유지하면서 index와 empty slot을 검사한다. |
| `Character` | `src/refactored/Character.*` | initialize, update, damage, death 순서를 고정한다. |
| `CharacterFactory` | `src/refactored/CharacterFactory.*` | `CharacterKind`에 맞는 concrete type과 기본 collaborator를 만든다. |
| `CharacterSlotStore` | `src/refactored/CharacterSlotStore.*` | acquire/release 상태와 reusable slot을 관리한다. |
| `CharacterMath` | `src/refactored/CharacterMath.*` | damage clamp, projectile count, snapshot 생성 같은 stateless 계산을 담당한다. |
| `PassiveRegistry` | `src/refactored/Passive/PassiveRegistry.*` | passive handler를 `PassiveTiming`별로 실행한다. |
| `ShotPattern` | `src/refactored/Projectile/ShotPattern.*` | projectile 생성 분기를 strategy object로 나눈다. |
| `TimedEffectList` | `src/refactored/Effects/TimedEffectList.*` | effect duration 감소와 expiration event 생성을 담당한다. |
| `ReturnRequestModule` | `src/refactored/Return/ReturnRequestModule.*` | return-request 관련 처리를 Character 밖으로 분리한다. |
| `AiDispatcher` | `src/refactored/AI/AiDispatcher.*` | AI 실행 지점을 별도 collaborator로 둔다. |
| `SimpleMemoryPool` | `src/refactored/Memory/SimpleMemoryPool.h` | allocation policy를 gameplay object 밖으로 분리하는 예제를 제공한다. |

## Legacy Function to Refactored Boundary

| Legacy example | Problem shown | Refactored boundary |
| --- | --- | --- |
| `LegacyAt`, `LegacyRawAt`, `gCharacters` | 전역 접근과 index 위험 | `CharacterAccessor` |
| `Initialize` 안의 registry 등록 | 생성과 접근 상태가 섞임 | `CharacterFactory`, `CharacterAccessor` |
| `Update` | AI, timer, passive, projectile, return-request가 한 함수에 있음 | `Character` lifecycle + collaborators |
| `ProcessAI` | 타입별 AI 분기가 Character 안에 있음 | `AiDispatcher` |
| `ApplyPassiveEffects` | 시점과 조건이 하나의 if-chain에 섞임 | `PassiveRegistry` |
| `Create*Projectile` | projectile 변형마다 좌표 계산이 반복됨 | `ShotPattern` |
| `UpdateStatusTimers`, `UpdateEffects` | timer 상태가 Character 멤버로 누적됨 | `TimedEffectList` |
| `ClampDamage`, `ProjectileCountForKind` | 계산 함수가 Character 내부에 있음 | `CharacterMath` |
| `SummonUnit`, `CleanupSummon` | 소유권과 lifetime 처리가 분산됨 | 별도 module로 분리할 대상 |

## Lifecycle Flow

```text
Refactored Character::Update(input)

  PreUpdate event
    |
    v
  OnPreUpdate(input)          // type hook
    |
    v
  AiDispatcher::Dispatch()
    |
    v
  TimedEffectList::Update()
    |
    v
  PassiveRegistry::Execute(Passive)
    |
    v
  ShotPattern::Emit()
    |
    v
  ReturnRequestModule::TryReturnRequest()
    |
    v
  PassiveRegistry::Execute(PostPassive)
    |
    v
  OnPostUpdate(input)         // type hook
    |
    v
  Updated event
```

이 흐름은 legacy `Update()`에 섞여 있던 실행 순서를 base class에 고정하고, 세부 처리를 모듈로 넘기는 구조를 보여준다.
