- 参考 Lyra，使用数据资产配置 Action

# InputConfig

基础代码与Lyra一致

- 使用自定义的`InputAction`，让每一个 Action 都跟一个`GamePlayTag`对应
- Config 继承自`DataAsset`，用于设置 Action 及其 Tag，以及角色用的`InputMappingContext`
- 同时有按 Tag 寻找 Action 的函数`FindNativeInputActionForTag`
    - 寻找逻辑就是简单遍历所有 Action 并对比 Tag
        ```cpp
        for (const FGAFInputAction& Action : NativeInputActions)
        {
            if (Action.InputAction && (Action.InputTag == InputTag))
            {
                return Action.InputAction;
            }
        }
        ```

---

# InputSettings

- 进行一些输入参数的配置，属于 Config 的成员变量
    - 比如灵敏度，操作方向反转

---

# BindingHelpers

- 对 BindAction 进行一层包装，可以按 GamePlayTagTag 进行绑定
    - 通过 `FindNativeInputActionForTag` 找到实际 Action