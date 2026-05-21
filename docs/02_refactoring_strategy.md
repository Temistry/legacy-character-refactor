# 02. Refactoring Strategy / 리팩토링 전략

## 전략 요약 / Strategy Summary

이 샘플의 전략은 점진적이고 동작 보존을 우선한다.
전체 시스템을 한 번에 교체하지 않고, 먼저 위험한 의존성 주변에 경계를 만든다.

The strategy is gradual and behavior-preserving.
The sample does not replace the whole system at once. It first places a boundary around dangerous dependencies.

## 1. 호출부 호환성 유지 / Keep Call-Site Compatibility

`CharacterAccessor`는 배열 스타일 접근 문법을 흡수한다.
기존 호출부를 한 번에 바꾸지 않고, invalid access 검사를 한 곳으로 모으는 역할을 한다.

`CharacterAccessor` absorbs array-like access syntax.
Old call sites can keep working while invalid access checks are centralized.

## 2. 실행 순서 보존 / Preserve Runtime Order

base `Character` class는 lifecycle 순서를 소유한다.
파생 클래스는 hook을 override할 수 있지만, 전체 update pipeline 순서를 쉽게 바꾸지 못한다.

The base `Character` class owns the lifecycle order.
Derived classes can override hooks, but they cannot accidentally reorder the whole update pipeline.

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

## 3. 상태 소유자 분리 / Move State to the Right Owner

effect duration은 `TimedEffectList`로 이동한다.
생성 책임은 `CharacterFactory`로 이동한다.
재사용 slot은 `CharacterSlotStore`가 관리한다.
passive 실행은 `PassiveRegistry`가 담당한다.

Effect durations move to `TimedEffectList`.
Creation moves to `CharacterFactory`.
Reusable slots move to `CharacterSlotStore`.
Passive execution moves to `PassiveRegistry`.

## 4. Data Object 단계 추가 / Add a Data Object Step

매직넘버와 skill id 기반 값은 먼저 code-level table로 모은다.
이 샘플에서는 `CharacterTuningTable`과 `SkillDefinitionTable`이 그 역할을 한다.

Magic values and skill id based values are first collected into code-level tables.
In this sample, `CharacterTuningTable` and `SkillDefinitionTable` provide that boundary.

## 5. 외부 관찰 결과 비교 / Compare External Observations

동작 보존은 내부 구현이 같은지로 판단하지 않는다.
state snapshot과 event trace를 비교해 health, energy, alive state, active effect count, event order를 확인한다.

Behavior preservation is not checked by comparing internal implementation.
The sample compares state snapshots and event traces for health, energy, alive state, active effect count, and event order.
