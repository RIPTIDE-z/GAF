using UnrealBuildTool;

public class GAF : ModuleRules
{
	public GAF(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		// Keep non-inlined generated source as a warning to speed up iteration
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
