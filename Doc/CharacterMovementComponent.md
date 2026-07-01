- 采用继承自 CMC 的自定义 CMC
- 通过覆写 `UpdateCharacterStateBeforeMovement` `PhysWalking` `PhysicsRotation` 来达到原GASP里 `Add Tick Prerequisite Component` 的作用
  - 按理根据CMC的更新顺序是可以做到在移动旋转开始前更新运动参数
  - 注意`Super::UpdateCharacterStateBeforeMovement` 会处理`Crouch`

