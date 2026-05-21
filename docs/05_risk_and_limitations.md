# 05. Risk and Limitations / 리스크와 한계

## 샘플 범위 / Sample Scope

이 저장소는 의도적으로 작은 toy project다.
목표는 전체 runtime 구현이 아니라, 리팩토링 판단 기준과 구조 분리 방식을 보여주는 것이다.

This repository is intentionally small.
It demonstrates refactoring judgment and boundary design, not a full runtime.

## 한계 / Limitations

- 실제 production source code를 포함하지 않는다. / No real production source code.
- 실제 game data를 포함하지 않는다. / No real game data.
- networking layer를 구현하지 않는다. / No networking layer.
- scripting layer를 구현하지 않는다. / No scripting layer.
- asset 또는 effect system을 구현하지 않는다. / No asset or effect system.
- 외부 content table parser를 구현하지 않는다. / No external content table parser.
- production allocator를 구현하지 않는다. / No production allocator.

## 실제 마이그레이션 리스크 / Remaining Risks in Real Migrations

실제 시스템에서는 update order, global state, random number sequence, frame timing read, logging order, ownership assumption, rare state를 따로 확인해야 한다.
테스트가 없는 rare state는 trace를 추가해 먼저 관찰 가능하게 만드는 것이 좋다.

A real migration must still watch update order, global state, random number sequence, frame timing reads, logging order, ownership assumptions, and rare states without tests.
Rare states should become observable through traces before behavior is moved.

## 실무 기준 / Practical Guideline

모든 호출부를 먼저 교체하지 않는다.
위험한 의존성 주변에 작은 경계를 만들고, trace 기반 검증을 붙인 뒤 내부 동작을 옮긴다.

Do not start by replacing every caller.
Place a small boundary around the dangerous dependency first, add trace-based validation, and then move behavior behind that boundary.
