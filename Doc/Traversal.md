# GAF Traversal

- `GAFCharacterTraversalComponent`
    - 挂在角色上，负责发起 Traversal 检测、维护 `bDoingTraversalAction`、后续选择 Montage 和执行翻越
- `GAFTraversableLedgeProviderComponent`
    - 挂在可翻越的场景 Actor 上。
    - 负责从该 Actor 的 ledge spline 中提供 Front / Back Ledge 数据
- `GAFTraversableLedgeSplineComponent`
    - 继承自 `USplineComponent`，表示一条可攀爬边缘，可直接在编辑器视口中编辑 spline 点
- `GAFTraversalTypes`
    - 存放 Traversal 使用的 enum 和 struct
    - 包括 `FGAFTraversalCheckInputs`、`FGAFTraversalCheckResult`、`EGAFTraversalActionType` 等
- `GAFTraversalCollisionResolver`
    - 从项目设置读取 Traversal 使用的 C++ `ECollisionChannel`
- `GAFTraversalSettings` 用于暴露 Project Settings 配置，可配置用于Traversal的碰撞频道

## 角色侧

`UGAFCharacterTraversalComponent` 由 `AGAFCharacterCore` 默认创建

- 输入逻辑在 `AGAFCharacterPlayable` 中处理：
    - Jump `Started`：先尝试 Traversal，失败后执行普通 `Jump()`
    - Jump `Triggered`：空中持续尝试 Traversal，失败不触发普通 Jump
    - Jump `Completed / Canceled`：调用 `StopJumping()`


## 场景侧

任意 Actor 只要添加 `UGAFTraversableLedgeProviderComponent`，就可以作为可翻越数据提供者，不需要继承特定 Actor 类

推荐组件组合：

```text
Actor
    UGAFTraversableLedgeProviderComponent
    UGAFTraversableLedgeSplineComponent
    UGAFTraversableLedgeSplineComponent
```

`UGAFTraversableLedgeProviderComponent` 内部通过 `LedgePairs` 配置边缘关系：

- 每个 pair 最多包含两条 `UGAFTraversableLedgeSplineComponent`
- 两条边没有固定 Front / Back 语义
- 查询时会根据角色位置自动选择离角色更近的一条作为 `FrontLedge`
- 允许只配置一条边，这种情况下只返回 FrontLedge
- `BeginPlay()` 会把编辑器中的 `FComponentReference` 解析成运行时弱引用缓存

## Ledge 查询逻辑

`GetLedgeTransforms()` 是场景侧的主要查询入口

当前流程：

1. 遍历所有已解析的 ledge pair
2. 对每个 pair 的两条边分别测试，选出更适合作为 FrontLedge 的一侧
3. 在所有 pair 中选择离角色最近的 ledge
4. 使用命中位置在 FrontLedge 上找最近点
5. 用 `MinLedgeWidth` 对该点沿 spline 的距离进行 clamp，避免位置太靠近 ledge 端点
6. 如果存在 BackLedge，则从 FrontLedgeLocation 出发，在 BackLedge 上找最近 transform 并写入结果

`MinLedgeWidth` 的作用是防止角色在 ledge 拐角或端点处翻越时悬空

## 数据类型

`FGAFTraversalCheckInputs` 表示一次 Traversal 检测所需的输入参数，例如 trace 方向、距离、胶囊半径和半高

`FGAFTraversalCheckResult` 表示检测结果，包括：

- Traversal 动作类型：`Hurdle`、`Vault`、`Mantle`
- Front / Back Ledge 数据
- Back Floor 数据
- 障碍物高度、深度和后边缘高度
- 命中的组件
- 选中的 Montage、StartTime、PlayRate
- 失败原因

## Trace Channel

Traversal 不硬编码 `ECC_GameTraceChannel1`，而是通过 `UGAFTraversalSettings` 在 Project Settings 中选择

推荐配置：

```text
Project Settings -> Engine -> Collision -> New Trace Channel
Name: Traversable
Default Response: Ignore
```

需要被 Traversal 检测命中的组件，把 `Traversable` 响应设置为 `Block`

然后在插件设置中选择：

```text
Project Settings -> Plugins -> GAF Traversal
Traversable Trace Channel: Traversable
```

C++ 检测时统一通过 resolver 获取 channel：

```cpp
const ECollisionChannel Channel =
	FGAFTraversalCollisionResolver::GetTraversalCollisionChannel();
```
