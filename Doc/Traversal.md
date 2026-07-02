# Traversal

仿照 GASP 的 `AC_TraversalLogic`，先尝试翻越，失败后再普通跳跃

- `UGAFTraversalComponent`挂在 `AGAFCharacterCore` 上
  - 不启用 Tick，只有输入触发时才执行 Traversal 尝试
  - 入口函数是 `TryTraversalAction()`
- `UGAFTraversalSettings`继承自 UDeveloperSettings 在 Project Settings 中暴露 Traversal 相关插件配置
  - 当前用于选择 Traversal Trace Channel
- `FGAFTraversalCollisionResolver`
  - 从 Project Settings 读取 C++ trace 使用的 `ECollisionChannel`

## Trace Channel 配置

不硬编码 ECC_GameTraceChannel1，通过 `UGAFTraversalSettings` 暴露配置

- TraversableTraceChannel

默认情况下，`TraversableTraceChannel` fallback 到 `Visibility`。这是为了保证插件初次接入时不会崩溃，但正式项目应该创建并选择专用 Trace Channel。

建议在项目中添加专用 Trace Channel：

```text
Project Settings -> Engine -> Collision -> New Trace Channel
Name: Traversable
Default Response: Ignore
```

然后配置需要被 Traversal 检测命中的组件：

```text
Traversable: Block
```

再到插件设置中选择该 Channel：

```text
Project Settings -> Plugins -> GAF Traversal
Traversable Trace Channel: Traversable
```

如果 `ECC_GameTraceChannel1` 已经被项目占用，可以用其他空闲槽位，例如 `ECC_GameTraceChannel2` 或 `ECC_GameTraceChannel3`。插件不依赖具体槽位，只依赖 Project Settings 里选择的 Trace Channel。

示例 ini：

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Traversable")
```

如果项目已经占用了 `ECC_GameTraceChannel1`，可以改成：

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel3,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Traversable")
```

## C++ 使用方式

Traversal trace 统一通过 resolver 获取 `ECollisionChannel`：

```cpp
const ECollisionChannel Channel =
	FGAFTraversalCollisionResolver::GetTraversalCollisionChannel();
```

后续 Traversal 检测建议使用底层 C++ trace，例如：

```cpp
const ECollisionChannel Channel =
	FGAFTraversalCollisionResolver::GetTraversalCollisionChannel();

GetWorld()->SweepSingleByChannel(
	Hit,
	Start,
	End,
	FQuat::Identity,
	Channel,
	Shape,
	QueryParams);
```

## Traversable Actor

这里对原本GASP的LevelBlock_Traversable进行拓展，“ 可攀爬 ” 功能不依赖特定 Actor 类。任意 Actor 只要添加以下组件即可参与后续 Traversal 检测：

- `UGAFTraversableLedgeSplineComponent`：实际可编辑的可攀爬边，可在同一个 Actor 上添加多个
- `UGAFTraversableComponent`：提供 `GetLedgeTransforms()` 查询入口
  - 内含 TraversableLedgePair 用于配对前后边缘
  - 使用 FComponentReference 以选择同一个Actor下的组件，并在运行时通过Resolver解析为指针
  - 当前会在BeginPlay进行一次整体解析

编辑器配置流程：

```text
Actor Blueprint
  Add Component -> GAF Traversable Component
  Add Component -> GAF Traversable Ledge Spline Component
  Add Component -> GAF Traversable Ledge Spline Component
```

在视口中直接编辑每条 `UGAFTraversableLedgeSplineComponent` 的 spline 点。然后在 `UGAFTraversableComponent` 的 `LedgePairs` 中显式配置 Front / Back 对应关系
