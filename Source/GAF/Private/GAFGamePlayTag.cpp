#include "GAFGamePlayTag.h"

namespace GAFGamePlayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Walk, "GAF.Gait.Walk", "Walking");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Run, "GAF.Gait.Run", "Running");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gait_Sprint, "GAF.Gait.Sprint", "Sprinting");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stance_Stand, "GAF.Gait.Stand", "Standing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stance_Crouch, "GAF.Gait.Crouch", "Crouching");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "GAF.InputTag.Move", "Move input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move_WorldSpace, "GAF.InputTag.Move.WorldSpace", "World-space move input, useful for debug");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Gamepad, "GAF.InputTag.Look.Gamepad", "Gamepad look input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "GAF.InputTag.Look.Mouse", "Mouse look input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Walk, "GAF.InputTag.Walk", "Walk input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "GAF.InputTag.Sprint", "Sprint input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "GAF.InputTag.Crouch", "Crouch input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "GAF.InputTag.Jump", "Jump input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "GAF.InputTag.Aim", "Aim input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_ChangeRotationMode, "GAF.InputTag.ChangeRotationMode", "Change rotation mode input");
} // namespace GAFGamePlayTags
