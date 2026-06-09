using UnrealBuildTool;

public class GAFPlayable : ModuleRules
{
	public GAFPlayable(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GAF"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EnhancedInput"
		});
	}
}
