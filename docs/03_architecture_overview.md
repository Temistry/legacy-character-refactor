# 03. Architecture Overview

The refactored sample separates responsibilities that were previously placed inside one class.

## Legacy View

```text
LegacyRegistry[index]
  -> LegacyCharacter
       -> Initialize
       -> Update
       -> ApplyDamage
       -> type switches
       -> passive branches
       -> projectile branches
       -> effect timer fields
```

## Refactored View

```text
CharacterAccessor[index]
  -> Character
       -> Template Method lifecycle
       -> virtual type hooks
       -> TimedEffectList
       -> PassiveRegistry
       -> ShotPattern
       -> ReturnRequestModule
       -> AiDispatcher

CharacterFactory
  -> concrete Character type
  -> default modules

CharacterSlotStore
  -> reusable ownership slots
```

## Responsibility Map

| Responsibility | Legacy sample | Refactored sample |
| --- | --- | --- |
| Access | global array | `CharacterAccessor` |
| Type difference | switch branches | virtual hooks |
| Creation | scattered assumptions | `CharacterFactory` |
| Reuse | implicit | `CharacterSlotStore` |
| Pure calculation | character methods | `CharacterMath` |
| Passive timing | if-chain | `PassiveRegistry` |
| Projectile shape | branch functions | `ShotPattern` |
| Effect timers | character fields | `TimedEffectList` |

> Korean note: 이 구조의 목적은 책임을 잘게 나누되, 실행 순서는 base lifecycle에서 통제하는 것입니다.
