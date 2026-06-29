using UnrealBuildTool;

public class GAFInput : ModuleRules
{
	public GAFInput(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"EnhancedInput",
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

	}
}
