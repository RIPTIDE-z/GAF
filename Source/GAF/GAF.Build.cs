using UnrealBuildTool;

public class GAF : ModuleRules
{
	public GAF(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"AnimGraphRuntime",
				"MotionWarping"
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

	}
}
