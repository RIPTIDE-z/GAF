#include "GAFGamePlayTag.h"

namespace GAFGamePlayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Walk, "GAF.Gait.Walk", "Walking");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Run, "GAF.Gait.Run", "Running");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Sprint, "GAF.Gait.Sprint", "Sprinting");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Stand, "GAF.Gait.Stand", "Standing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Crouch, "GAF.Gait.Crouch", "Crouching");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move_WorldSpace, "InputTag.Move.WorldSpace", "World-space move input, useful for debug");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Gamepad, "InputTag.Look.Gamepad", "Gamepad look input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Mouse look input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Walk, "InputTag.Walk", "Walk input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "InputTag.Sprint", "Sprint input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "InputTag.Aim", "Aim input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_ChangeRotationMode, "InputTag.ChangeRotationMode", "Change rotation mode input");
} // namespace GAFGamePlayTags