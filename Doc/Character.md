- 使用一个 CharacterCore 编写主要逻辑，Playable 负责输入配置等

# Tick Prerequisite

- GASP 里会使用一个组件 AC_PreTick 以及 `Add Tick Prerequisite Component` `Bind Event to Tick` 来让角色数据获取和更新在 CMC 更新之前
- 这是因为蓝图不能操作底层 `TickFuntion`，想要编排 Tick 顺序只能依赖 Actor/Component 的 TickFunction
- C++ 则可以直接调用

