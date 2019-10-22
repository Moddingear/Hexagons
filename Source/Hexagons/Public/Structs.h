#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RuntimeMeshGenericVertex.h"
#include "Structs.generated.h"

DECLARE_RUNTIME_MESH_VERTEX(FHexVertex, true, false, false, true, 1, ERuntimeMeshVertexTangentBasisType::Default, ERuntimeMeshVertexUVType::Default)

USTRUCT(BlueprintType)
struct FHexObstacle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) //Use the time and speed to get the position of the obstacle
	float TimeCreated;

	UPROPERTY(BlueprintReadWrite)
	float Speed; //in cm/s

	UPROPERTY(BlueprintReadWrite)
	float Thickness; //in cm

	UPROPERTY(BlueprintReadWrite) //There should be less than 256 sides, and they couldn't be negative, hence unsigned
		uint8 Side;

	FHexObstacle()
		:TimeCreated(0),
		Speed(0.f),
		Thickness(0.f),
		Side(0)
	{}

	FHexObstacle(FDateTime TimeCreatedIn, float SpeedIn, float ThicknessIn, uint8 SideIn)
		:TimeCreated(TimeCreated),
		Speed(SpeedIn),
		Thickness(ThicknessIn),
		Side(SideIn)
	{}

	float GetDistance(UObject* WorldContextObject) const
	{
		float DeltaTime = UKismetSystemLibrary::GetGameTimeInSeconds(WorldContextObject) - TimeCreated;
		return DeltaTime * Speed;
	}
};