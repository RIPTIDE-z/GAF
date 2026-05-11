using UnrealBuildTool;

public class GAFALS : ModuleRules
{
	public GAFALS(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		// .gen.cpp 没有被内联进对应 .cpp 只进行Warning，加快编译速度
		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;

		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"AnimGraphRuntime",
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

	}
}
