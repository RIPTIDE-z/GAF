# GAF Traversal

## 模块概览

- `GAFCharacterTraversalComponent`
  - 挂在角色上，负责发起 Traversal 检测、维护 `bDoingTraversalAction`、选择 Montage，并为后续执行翻越提供结果
- `GAFTraversableLedgeProviderComponent`
  - 挂在可翻越的场景 Actor 上，负责从该 Actor 的 ledge spline 中提供 Front / Back Ledge 数据
- `GAFTraversableLedgeSplineComponent`
  - 继承自 `USplineComponent`，表示一条可攀爬边缘，可直接在编辑器视口中编辑 spline 点
- `UGAFAnimationInstance`
  - 保存 Traversal Interaction Transform，并通过 PoseHistory Tag 获取 `FPoseHistoryReference`
- `GAFTraversalTypes`
  - 存放 Traversal 使用的 enum 和 struct
  - 包括 `FGAFTraversalCheckInputs`、`FGAFTraversalCheckResult`、`FGAFTraversalChooserInputs`、`FGAFTraversalChooserOutputs`、`EGAFTraversalActionType` 等
- `GAFTraversalCollisionResolver`
  - 解析 Traversal 使用的 C++ `ECollisionChannel`
- `FGAFTraversalSettings`
  - 嵌在 `UGAFCharacterSettings` 中，配置角色侧 Traversal trace、debug 和可选 collision channel override
- `FGAFMotionMatchingSettings`
  - 嵌在 `UGAFCharacterSettings` 中，配置 Traversal Chooser、PoseHistory Tag 和默认 PlayRate
- `UGAFTraversalConfig`
  - Project Settings 中的全局 Traversal 配置，提供默认 collision channel

## 角色侧

`UGAFCharacterTraversalComponent` 由 `AGAFCharacterCore` 默认创建。

- `AGAFCharacterCore::GetTraversalCheckInputs()` 会从 `CharacterSettings.TraversalSettings` 构造本次检测输入
- `MOVE_Falling / MOVE_Flying` 使用空中 trace 配置
- 其他 MovementMode 使用地面 trace 配置，并根据角色本地前向速度放大检测距离
- 输入逻辑在 `AGAFCharacterPlayable` 中处理：
  - Jump `Started`：先尝试 Traversal，失败后执行普通 `Jump()`
  - Jump `Triggered`：空中持续尝试 Traversal，失败不触发普通 Jump
  - Jump `Completed / Canceled`：调用 `StopJumping()`

`TryTraversalAction()` 不从组件内部读取角色配置，而是由调用方显式传入当前角色的配置：

```cpp
CharacterTraversalComponent->TryTraversalAction(
	GetTraversalCheckInputs(),
	GetDefaultCharacterSettings().TraversalSettings,
	GetDefaultCharacterSettings().MotionMatchingSettings,
	EDrawDebugTrace::ForDuration,
	TraversalCheckResult);
```

## 场景侧

任意 Actor 只要添加 `UGAFTraversableLedgeProviderComponent`，就可以作为可翻越数据提供者，不需要继承特定 Actor 类。

推荐组件组合：

```text
Actor
    UGAFTraversableLedgeProviderComponent
    UGAFTraversableLedgeSplineComponent
    UGAFTraversableLedgeSplineComponent
```

`UGAFTraversableLedgeProviderComponent` 内部通过 `LedgePairs` 配置边缘关系：

- 每个 pair 最多包含两条 `UGAFTraversableLedgeSplineComponent`
- 每条边可以单独配置是否可攀爬
- 两条边没有固定 Front / Back 语义
- 查询时会根据角色位置自动选择离角色更近的一条作为 `FrontLedge`
- 允许只配置一条边，这种情况下只返回 FrontLedge
- `BeginPlay()` 会把编辑器中的 `FComponentReference` 解析成运行时弱引用缓存

## 检测流程

`UGAFCharacterTraversalComponent::TryTraversalAction()` 的当前职责是完成检测和 Montage 选择，实际播放 Montage / MotionWarping 的 Step 5 仍是后续逻辑。

主要流程：

1. 从 Character 通过 `IGAFCharacterDataProvider::GetTraversalFrameData()` 获取角色数据
2. 用 `FGAFTraversalCheckInputs` 做前方 capsule trace，寻找带 `UGAFTraversableLedgeProviderComponent` 的 Actor
3. 调用 provider 的 `GetLedgeTransforms()` 写入 Front / Back Ledge
4. 检查角色是否有空间移动到 FrontLedge
5. 计算 `ObstacleHeight`
6. 如果存在 BackLedge，检查顶部空间并计算 `ObstacleDepth`
7. 从 BackLedge 向下检测 BackFloor，并计算 `BackLedgeHeight`
8. 把 FrontLedge 转成 Interaction Transform 写入 AnimInstance
9. 构造 ChooserInput，读取 PoseHistory，评估 Traversal Chooser
10. 将 Chooser 输出写回 `FGAFTraversalCheckResult`
11. 打印 Traversal 条件和 Chooser 结果

## Ledge 查询逻辑

`GetLedgeTransforms()` 是场景侧的主要查询入口。

当前流程：

1. 遍历所有已解析的 ledge pair
2. 对每个 pair 的两条边分别测试，选出更适合作为 FrontLedge 的一侧
3. 在所有 pair 中选择离角色最近的 ledge
4. 使用命中位置在 FrontLedge 上找最近点
5. 用 `MinLedgeWidth` 对该点沿 spline 的距离进行 clamp，避免位置太靠近 ledge 端点
6. 如果存在 BackLedge，则从 `FrontLedgeLocation` 出发，在 BackLedge 上找最近 transform 并写入结果

`MinLedgeWidth` 的作用是防止角色在 ledge 拐角或端点处翻越时悬空。

## Motion Matching

Traversal 的 Montage 选择不写死在 C++ 里，而是通过 `FGAFMotionMatchingSettings` 引用 Chooser 资产。

主要参数：

- `TraversalMontageChooser`：Traversal 使用的 `UChooserTable`，负责根据检测结果和 PoseHistory 选择 Montage
- `TraversalPoseHistoryTag`：AnimGraph 中 Pose Search History Collector 节点的 Tag
- `DefaultTraversalPlayRate`：Chooser 成功后写入结果的默认播放速率

Step 4.1 会把 `FrontLedgeLocation` 和 `FrontLedgeNormal` 转换成 Interaction Transform：

```text
Location = FrontLedgeLocation
Rotation = MakeRotFromZ(FrontLedgeNormal)
Scale = (1, 1, 1)
```

该 Transform 会写入 `UGAFAnimationInstance`，供 Motion Matching 的自定义 channel 使用。

Step 4.2 会构造 `FGAFTraversalChooserInputs`，然后评估 `TraversalMontageChooser`。成功后将 `FGAFTraversalChooserOutputs` 写回 `FGAFTraversalCheckResult`：

- `ActionType`
- `ChosenMontage`
- `StartTime`
- `PlayRate`

`ChosenMontage` 优先读取 Chooser output struct。如果 output struct 中没有 Montage，则尝试把 Chooser 的 Result Object 转成 `UAnimMontage`，兼容 UE Chooser 常规的 Result Object 用法。

当前不区分 GASP 中的 Mover / CMC 分支，所有角色统一使用同一个 Chooser 流程。

## PoseHistory

蓝图中常见做法是通过实验性的 Anim Node Reference 节点转换得到 PoseHistory。C++ 里更推荐使用 PoseHistory Tag 查找节点：

```cpp
const FAnimNode_PoseSearchHistoryCollector_Base* PoseHistoryNode =
	UPoseSearchLibrary::FindPoseHistoryNode(PoseHistoryTag, AnimInstance);
```

找到节点后，先检查 `GetPoseHistoryPtr()` 是否有效，再通过 `GetPoseHistoryReference()` 传给 Chooser。

这个逻辑放在 `UGAFAnimationInstance` 内部更合适：

- Traversal 组件只关心是否能拿到 `FPoseHistoryReference`
- AnimInstance 负责知道自己的 AnimGraph 中哪个 PoseHistory Tag 可用
- 以后替换 AnimGraph 节点或 Tag 时，只需要改 `MotionMatchingSettings.TraversalPoseHistoryTag`

## 数据类型

`FGAFTraversalCheckInputs` 表示一次 Traversal 检测所需的输入参数，例如 trace 方向、距离、胶囊半径和半高。

`FGAFTraversalCheckResult` 表示检测结果，包括：

- Traversal 动作类型：`Hurdle`、`Vault`、`Mantle`
- Front / Back Ledge 数据
- Back Floor 数据
- 障碍物高度、深度和后边缘高度
- 命中的组件
- 选中的 Montage、StartTime、PlayRate
- 失败原因

常见失败原因：

- `InvalidAnimInstance`：角色 Mesh 上没有可用的 `UGAFAnimationInstance`
- `InvalidPoseHistory`：指定的 PoseHistory Tag 找不到节点，或节点内部历史数据无效
- `InvalidTraversalChooser`：`MotionMatchingSettings.TraversalMontageChooser` 未配置
- `MontageSelectionFailed`：Chooser 没有输出有效 Montage

`FGAFTraversalChooserInputs` 表示传给 Chooser 的输入数据：

- Traversal 检测条件
- `DistanceToLedge`
- `MovementMode`
- `Gait`
- `Speed`
- `PoseHistory`

`FGAFTraversalChooserOutputs` 表示 Chooser 写出的结果：

- `ActionType`
- `ChosenMontage`
- `MontageStartTime`

## Traversal Settings

角色侧 Traversal 参数配置在 `UGAFCharacterSettings` 的 `TraversalSettings` 内，和 `MovementSettings`、`MotionMatchingSettings` 同级。

主要参数：

- `TraceCapsuleRadius`：前方胶囊检测半径
- `GroundTraceCapsuleHalfHeight`：地面检测胶囊半高
- `GroundMinTraceForwardDistance`：地面低速时的最小前方检测距离
- `GroundMaxTraceForwardDistance`：地面高速时的最大前方检测距离
- `GroundMaxTraceForwardSpeed`：本地前向速度达到该值时使用最大检测距离
- `AirTraceForwardDistance`：空中固定前方检测距离
- `AirTraceEndOffset`：空中 trace 终点偏移，当前默认向上偏移 50
- `AirTraceCapsuleHalfHeight`：空中检测胶囊半高
- `BackFloorTraceExtraDistance`：BackFloor 向下检测额外距离，避免另一侧地面略低时漏检
- `DebugDrawLevel`：trace debug 等级，当前 `0/1` 不绘制，`2+` 按传入的 `EDrawDebugTrace::Type` 绘制
- `DebugDrawDuration`：持续绘制时的保留时间
- `DebugPrintColor`：Traversal 屏幕调试文本颜色
- `DebugPrintDuration`：Traversal 屏幕调试文本显示时间

默认数值复现当前 GASP 逻辑：

```text
Ground:
TraceCapsuleRadius = 30
TraceCapsuleHalfHeight = 60
ForwardDistance = LocalForwardSpeed 0..500 -> 75..350

Air:
TraceCapsuleRadius = 30
TraceCapsuleHalfHeight = 86
ForwardDistance = 75
TraceEndOffset = (0, 0, 50)
```

## Trace Channel

Traversal 不硬编码 `ECC_GameTraceChannel1`，而是通过配置解析最终使用的 `ECollisionChannel`。

推荐配置：

```text
Project Settings -> Engine -> Collision -> New Trace Channel
Name: Traversable
Default Response: Ignore
```

需要被 Traversal 检测命中的组件，把 `Traversable` 响应设置为 `Block`。

然后在插件设置中选择：

```text
Project Settings -> Plugins -> GAF Traversal Config
Traversable Trace Channel: Traversable
```

默认情况下，C++ 检测通过 `UGAFTraversalConfig` 读取全局 channel：

```cpp
const ECollisionChannel Channel =
	FGAFTraversalCollisionResolver::GetTraversalCollisionChannel();
```

如果某个角色需要特殊 channel，可以在 `CharacterSettings.TraversalSettings` 中开启：

```text
bOverrideTraversalTraceChannel = true
TraversalTraceChannel = 该角色专用 Channel
```

此时 resolver 会优先使用角色配置：

```cpp
const ECollisionChannel Channel =
	FGAFTraversalCollisionResolver::GetTraversalCollisionChannel(&TraversalSettings);
```

优先级：

```text
CharacterSettings.TraversalSettings override
-> Project Settings: UGAFTraversalConfig
-> ECC_Visibility fallback
```

## Debug

Traversal Debug 分为两类：

- 失败原因打印：`FailureReason` 由 `TryTraversalAction()` 写入 `FGAFTraversalCheckResult`，失败时统一打印
- 检测结果打印：成功进入 Chooser 后，打印 ledge / obstacle / montage 相关状态

屏幕打印颜色和持续时间来自：

```text
TraversalSettings.DebugPrintColor
TraversalSettings.DebugPrintDuration
```

Trace 和 ledge 绘制由 `DebugDrawLevel` 控制：

```text
0 / 1: 不绘制 trace
2: 绘制主要 Traversable search trace
3+: 绘制 HasRoom / TopSweep / BackFloor 等详细 trace
```

FrontLedge debug sphere 使用绿色，BackLedge debug sphere 使用蓝色。
