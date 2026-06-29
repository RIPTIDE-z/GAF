using UnrealBuildTool;

public class GAFPlayable : ModuleRules
{
	public GAFPlayable(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GAF",
			"GAFInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EnhancedInput"
		});
	}
}
