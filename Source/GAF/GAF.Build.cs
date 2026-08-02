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
				"DeveloperSettings",
				"GameplayTags",
				"AnimGraphRuntime",
				"AnimationWarpingRuntime",
				"MotionWarping",
				"Chooser",
				"PoseSearch"
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

	}
}
