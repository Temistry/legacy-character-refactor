cmake_minimum_required(VERSION 3.10)

project(BehaviorPreservingCharacterRefactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(character_sample STATIC
    src/legacy/LegacyCharacter.cpp
    src/refactored/AI/AiDispatcher.cpp
    src/refactored/Character.cpp
    src/refactored/CharacterAccessor.cpp
    src/refactored/CharacterFactory.cpp
    src/refactored/CharacterMath.cpp
    src/refactored/CharacterSlotStore.cpp
    src/refactored/Data/CharacterTuningTable.cpp
    src/refactored/Data/SkillDefinitionTable.cpp
    src/refactored/Effects/TimedEffectList.cpp
    src/refactored/Passive/PassiveRegistry.cpp
    src/refactored/Projectile/ShotPattern.cpp
    src/refactored/Return/ReturnRequestModule.cpp
    src/refactored/Skill/SkillExecutor.cpp
)

target_include_directories(character_sample PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

function(add_sample_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE character_sample)
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_sample_test(CharacterEquivalenceTests tests/CharacterEquivalenceTests.cpp)
add_sample_test(CharacterAccessorTests tests/CharacterAccessorTests.cpp)
add_sample_test(CharacterSlotStoreTests tests/CharacterSlotStoreTests.cpp)
add_sample_test(RefactoredModuleDetailTests tests/RefactoredModuleDetailTests.cpp)
add_sample_test(VirtualHookOrderTests tests/VirtualHookOrderTests.cpp)
