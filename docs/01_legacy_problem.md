# 01. Legacy Problem

This document describes the problem shape represented by the toy legacy sample. It does not describe a specific commercial codebase.

> Korean note: 이 문서는 특정 회사 코드가 아니라, 오래된 C++ 게임 클라이언트에서 흔히 나타나는 구조적 문제를 일반화한 것입니다.

## Giant Character Class

The legacy sample puts lifecycle, damage, passive behavior, projectile behavior, effect timers, and return-request behavior into `LegacyCharacter`. This makes every change risky because a small modification can affect unrelated behavior.

## Global Array Access

Old call sites often assume that character objects can be reached through a global array-like registry. That is convenient, but it spreads lifetime and validity assumptions across the codebase.

## Type Branch Accumulation

The legacy sample uses `switch (CharacterKind)` to express type differences. This works for a small number of types, but it becomes fragile when every new type adds another exception.

## Mixed Responsibilities

The main issue is not that the code is old. The issue is that unrelated reasons to change are packed into the same class.

> Korean note: 리팩토링의 출발점은 못생긴 코드가 아니라 변경 이유가 한 곳에 너무 많이 모여 있는 구조입니다.
