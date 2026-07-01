- 使用一个 CharacterCore 编写主要逻辑，Playable 负责输入配置等

# Tick Prerequisite

- GASP 里会使用一个组件 AC_PreTick 以及 `Add Tick Prerequisite Component` `Bind Event to Tick` 来让角色数据获取和更新在 CMC 更新之前

# Playable

- 在`PostInitializeComponents`里做组件默认配置，防止CDO阶段读取到空`DataAsset`导致崩溃
    - `InitCharacterMovementSettings` 读取配置信息
- 覆写`SetupPlayerInputComponent`来做按键绑定
- 仿照 ALS Refactor 在 `NotifyControllerChanged` 绑定 IMC