#include "Utility/GAFGamePlayTag.h"

namespace GAFGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{ TEXTVIEW("GAF.Gait.Walking") });
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{ TEXTVIEW("GAF.Gait.Running") });
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{ TEXTVIEW("GAF.Gait.Sprinting") });
} // namespace GAFGaitTags

namespace GAFStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{ TEXTVIEW("GAF.Gait.Standing") });
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{ TEXTVIEW("GAF.Gait.Crouching") });
} // namespace GAFGaitTags