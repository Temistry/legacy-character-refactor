# 02. Refactoring Strategy

The strategy is gradual and behavior-preserving. The sample does not replace the whole system at once.

> Korean note: 이 샘플의 전략은 전체 재작성보다 호환 계층을 먼저 세우고 내부를 점진적으로 교체하는 방식입니다.

## 1. Keep Call-Site Compatibility

`CharacterAccessor` absorbs array-like access syntax. This allows old call sites to keep working while invalid access checks are centralized.

## 2. Preserve Runtime Order

The base `Character` class owns the lifecycle order. Derived classes can override hooks, but they cannot reorder the whole update pipeline by accident.

```text
Initialize
OneShot passive
PreUpdate hook
AI dispatch
Effect update
Passive timing
Projectile strategy
Return-request module
PostPassive timing
PostUpdate hook
Updated event
Damage
Dying passive
Death
```

## 3. Move State to the Right Owner

Effect durations move to `TimedEffectList`. Creation moves to `CharacterFactory`. Reusable slots move to `CharacterSlotStore`. Passive execution moves to `PassiveRegistry`.

## 4. Compare External Observations

Behavior preservation is tested through state snapshots and event traces. The sample checks health, energy, alive state, active effect count, and event order.

> Korean note: 리팩토링 검증은 내부 구현이 같은지 보는 것이 아니라, 외부에서 관찰되는 결과가 같은지 보는 것입니다.
