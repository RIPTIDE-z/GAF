#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAFVector.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GAF_API UGAFVector : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Clamp vector length to a maximum of 1
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector ClampMagnitude01(const FVector& Vector);

	static FVector3f ClampMagnitude01(FVector3f Vector);

	// Clamp 2D vector length to a maximum of 1
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", DisplayName = "Clamp Magnitude 01 2D", Meta = (ReturnDisplayName = "Vector"))
	static FVector2D ClampMagnitude012D(FVector2D Vector);

	// Convert radians to a 2D direction
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector2D RadianToDirection(float Radian);

	// Convert radians to an XY-plane direction
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector RadianToDirectionXY(float Radian);

	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector2D AngleToDirection(float Angle);

	// Convert degrees to an XY-plane direction
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector AngleToDirectionXY(float Angle);

	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngle(FVector2D Direction);

	// Convert an XY-plane direction to degrees
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngleXY(const FVector& Direction);

	// Return the vector rotated 90 degrees clockwise in the XY plane
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularClockwiseXY(const FVector& Vector);

	// Return the vector rotated 90 degrees counterclockwise in the XY plane
	UFUNCTION(BlueprintPure, Category = "GAF|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularCounterClockwiseXY(const FVector& Vector);
};

inline FVector UGAFVector::ClampMagnitude01(const FVector& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale };
}

inline FVector3f UGAFVector::ClampMagnitude01(const FVector3f Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale };
}

inline FVector2D UGAFVector::ClampMagnitude012D(const FVector2D Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale };
}

inline FVector2D UGAFVector::RadianToDirection(const float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return { Cos, Sin };
}

inline FVector UGAFVector::RadianToDirectionXY(const float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return { Cos, Sin, 0.0f };
}

inline FVector2D UGAFVector::AngleToDirection(const float Angle)
{
	// Convert degrees to radians before building the direction
	return RadianToDirection(FMath::DegreesToRadians(Angle));
}

inline FVector UGAFVector::AngleToDirectionXY(const float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

inline double UGAFVector::DirectionToAngle(FVector2D Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

inline double UGAFVector::DirectionToAngleXY(const FVector& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

inline FVector UGAFVector::PerpendicularClockwiseXY(const FVector& Vector)
{
	return { Vector.Y, -Vector.X, Vector.Z };
}

inline FVector UGAFVector::PerpendicularCounterClockwiseXY(const FVector& Vector)
{
	return { -Vector.Y, Vector.X, Vector.Z };
}
