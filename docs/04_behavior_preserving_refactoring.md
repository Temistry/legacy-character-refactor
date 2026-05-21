# 04. Behavior-Preserving Refactoring

Behavior preservation means more than returning the same final value. For game client code, visible behavior can depend on ordering.

> Korean note: 게임 클라이언트 리팩토링에서는 결과값뿐 아니라 순서가 중요합니다.

## Observations to Preserve

The sample focuses on these observable outputs:

- return value
- state change
- event order
- effect trigger order
- death handling order

In a production migration, additional observations can matter:

- log order
- sound trigger order
- visual effect trigger order
- random call order
- timing and frame read order

## Compatibility Layer

`CharacterAccessor` keeps array-like access while introducing validation. This reduces the need for a large caller rewrite.

## Template Method Lifecycle

The base class owns lifecycle order. Derived types can customize behavior through hooks, but they do not own the entire algorithm.

## Snapshot and Trace Tests

The tests compare `CharacterStateSnapshot`, `std::vector<CharacterEvent>`, reusable slot behavior, accessor failure behavior, and virtual hook call order.

> Korean note: 동치 보존 테스트는 새 구조가 더 좋아 보인다보다 같은 입력에서 같은 관찰 결과를 낸다를 확인합니다.
