#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Structs.generated.h"

USTRUCT(BlueprintType)
struct FHexObstacle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) //Use the time and speed to get the position of the obstacle
	FDateTime TimeCreated;

	UPROPERTY(BlueprintReadWrite)
	float Distance;

	UPROPERTY(BlueprintReadWrite)
	float Speed; //in cm/s

	UPROPERTY(BlueprintReadWrite)
	float Thickness; //in cm

	UPROPERTY(BlueprintReadWrite) //There should be less than 256 sides, and they couldn't be negative, hence unsigned
	uint8 Side;

	FHexObstacle()
		:TimeCreated(0),
		Distance(0.0),
		Speed(0.f),
		Thickness(0.f),
		Side(0)
	{}

	FHexObstacle(FDateTime TimeCreatedIn, float DistanceIn, float SpeedIn, float ThicknessIn, uint8 SideIn)
		:TimeCreated(TimeCreatedIn),
		Distance(DistanceIn),
		Speed(SpeedIn),
		Thickness(ThicknessIn),
		Side(SideIn)
	{}

	float GetDistance(UObject* WorldContextObject) const
	{
		FTimespan DeltaTime = FDateTime::UtcNow() - TimeCreated;
		return Distance - DeltaTime.GetTotalSeconds() * Speed;
	}

	float GetDistance(FDateTime CurrentTime) const
	{
		FTimespan DeltaTime = CurrentTime - TimeCreated;
		return Distance - DeltaTime.GetTotalSeconds() * Speed;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("{TimeCreated : {%s}, Distance : {%f}, Speed : {%f}, Thickness : {%f}, Side : {%i}}"), *TimeCreated.ToString(), Distance, Speed, Thickness, Side);
	}
};

USTRUCT(BlueprintType)
struct FHexRenderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		TArray<FHexObstacle> ObstaclesToRender;
	UPROPERTY(BlueprintReadWrite)
		FColor ObstacleColor;
	UPROPERTY(BlueprintReadWrite)
		FColor ObstacleEdgeColor;
	UPROPERTY(BlueprintReadWrite)
		float EdgeProportion;

	UPROPERTY(BlueprintReadWrite)
		float Sides;

	UPROPERTY(BlueprintReadWrite)
		float CoreLength;
	UPROPERTY(BlueprintReadWrite)
		FColor CoreColor;

	UPROPERTY(BlueprintReadWrite)
		float FloorLength;
	UPROPERTY(BlueprintReadWrite)
		float FloorDistance;
	UPROPERTY(BlueprintReadWrite)
		FColor FloorColorEven;
	UPROPERTY(BlueprintReadWrite)
		FColor FloorColorOdd;

	UPROPERTY(BlueprintReadWrite)
		UMaterialInterface* Material;
};