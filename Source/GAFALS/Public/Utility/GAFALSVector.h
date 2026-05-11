#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAFALSVector.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GAFALS_API UGAFALSVector : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 将向量最大长度限制为1
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector ClampMagnitude01(const FVector& Vector);

	static FVector3f ClampMagnitude01(FVector3f Vector);

	// 将2维向量最大长度限制为1
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", DisplayName = "Clamp Magnitude 01 2D", Meta = (ReturnDisplayName = "Vector"))
	static FVector2D ClampMagnitude012D(FVector2D Vector);

	// 把弧度角转换成方向向量
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector2D RadianToDirection(float Radian);

	// 弧度 -> XY 平面方向向量
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector RadianToDirectionXY(float Radian);

	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector2D AngleToDirection(float Angle);

	// 角度值（度）-> XY 平面方向向量
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (ReturnDisplayName = "Direction"))
	static FVector AngleToDirectionXY(float Angle);

	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngle(FVector2D Direction);

	// XY 平面方向向量 -> 角度值（通常是度）
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngleXY(const FVector& Direction);

	// 返回这个向量在 XY 平面内顺时针旋转 90 度后的垂直向量
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularClockwiseXY(const FVector& Vector);

	// 返回这个向量在 XY 平面内逆时针旋转 90 度后的垂直向量
	UFUNCTION(BlueprintPure, Category = "GAFALS|Vector Utility", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularCounterClockwiseXY(const FVector& Vector);
};

inline FVector UGAFALSVector::ClampMagnitude01(const FVector& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale };
}

inline FVector3f UGAFALSVector::ClampMagnitude01(const FVector3f Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale };
}

inline FVector2D UGAFALSVector::ClampMagnitude012D(const FVector2D Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return { Vector.X * Scale, Vector.Y * Scale };
}

inline FVector2D UGAFALSVector::RadianToDirection(const float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return { Cos, Sin };
}

inline FVector UGAFALSVector::RadianToDirectionXY(const float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return { Cos, Sin, 0.0f };
}

inline FVector2D UGAFALSVector::AngleToDirection(const float Angle)
{
	// 先转弧度再转向量
	return RadianToDirection(FMath::DegreesToRadians(Angle));
}

inline FVector UGAFALSVector::AngleToDirectionXY(const float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

inline double UGAFALSVector::DirectionToAngle(FVector2D Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

inline double UGAFALSVector::DirectionToAngleXY(const FVector& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

inline FVector UGAFALSVector::PerpendicularClockwiseXY(const FVector& Vector)
{
	return { Vector.Y, -Vector.X, Vector.Z };
}

inline FVector UGAFALSVector::PerpendicularCounterClockwiseXY(const FVector& Vector)
{
	return { -Vector.Y, Vector.X, Vector.Z };
}
