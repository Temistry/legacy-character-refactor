# 05. Risk and Limitations

This repository is intentionally small. It demonstrates refactoring judgment, not a full runtime.

## Limitations

- no real production source code
- no real game data
- no networking
- no scripting layer
- no asset or effect system
- no content table parsing
- no production allocator

## Remaining Risks in Real Migrations

Even with this approach, a real migration must watch for hidden dependency on update order, global state, random number sequence, frame timing reads, logging order, ownership assumptions, and rare states without tests.

## Practical Guideline

Do not start by replacing every caller. Start by placing a boundary around the dangerous dependency, then move behavior behind that boundary with trace-based validation.

> Korean note: 가장 위험한 선택은 구조가 나쁘다는 이유만으로 한 번에 전체를 갈아엎는 것입니다.
