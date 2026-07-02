Character CMC AnimInstance 之间的数据传递，初版尽可能仿照GASP中通过**接口**传递的方式

- 传递所用数据结构位于 `AnimationTypes`，分为 `AnimationFrameData` `CameraFrameData` `TraversalFrameData`
  - 以及用于 CMC 传递数据的 `LocomotionData`
- 接口位于 `CharacterDataProvider`，含有四个数据获取接口函数，并让 CharacterCore 继承两个接口类
- `IGAFCharacterDataProvider`
  - `GetAnimationFrameData`
  - `GetCameraFrameData`
  - `GetTraversalFrameData`
- `IGAFLocomotionDataProvider`

<br>

- 同时把一些状态类枚举转为 `GamePlayTags`，比如 InputState 转为 `TagContainer`
- CharacterCore 再继承接口并实现 `GetAnimationData` `GetMovementData`
  - 其中 GetAnimationData 调用 `BuildAnimationData` 进行实际的数据计算
- CMC 在 `UpdateCharacterStateBeforeMovement` 调用 `GetLocomotionData` 进行更新
  - 其中更新的数据先存入缓存 `CachedLocomotionData`
  - 到真正进行移动/旋转更新时再读取缓存数据
- 注意这个方式不能使用 `GetPendingInputVector`，因为是在CMC更新途中获取数据，在之前会消耗 `InputVector`，需要调用 `GetLastInputVector` 来获取消耗的Input，不然会始终取到`(0, 0, 0)`
    - 见 `UCharacterMovementComponent::TickComponent` 
        ```cpp
        UCharacterMovementComponent::TickComponent()
        {
            if (!bUsingAsyncTick)
            {
                // 这里会清空 PendingMovementInputVector
                InputVector = ConsumeInputVector();
            }

            ...

            // 这里面会调用UpdateCharacterStateBeforeMovement
            ControlledCharacterMove(InputVector, DeltaTime);
        }
        ```