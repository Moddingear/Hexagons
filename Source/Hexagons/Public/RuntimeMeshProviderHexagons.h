// Copyright 2016-2019 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "Structs.h"
#include "RuntimeMeshProviderHexagons.generated.h"

class HEXAGONS_API FRuntimeMeshProviderHexagonsProxy : public FRuntimeMeshProviderProxy
{
	TArray<FHexObstacle> ObstaclesToRender;
	FColor ObstacleColor;

	float Sides;

	float CoreLength;
	FColor CoreColor;

	float FloorLength;
	float FloorDistance;
	FColor FloorColor;

	UMaterialInterface* Material;

	float RenderTime;

private:
	int32 AddVertex(FRuntimeMeshRenderableMeshData& MeshData, FVector location, FColor color);

public:
	FRuntimeMeshProviderHexagonsProxy(TWeakObjectPtr<URuntimeMeshProvider> InParent, UMaterialInterface* InMaterial, 
		TArray<FHexObstacle> InObstaclesToRender, FColor InObstacleColor, float InSides, float InCoreLength, FColor InCoreColor, float InFloorLength, float InFloorDistance, FColor InFloorColor, float InRenderTime);
	~FRuntimeMeshProviderHexagonsProxy();

	virtual void Initialize() override;

	virtual bool GetSectionMeshForLOD(uint8 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;

	FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	bool HasCollisionMesh() override;
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData) override;

	virtual FBoxSphereBounds GetBounds() override { return FBoxSphereBounds(FSphere(FVector::ZeroVector, FloorLength)); }

	bool IsThreadSafe() const override;

};

UCLASS(HideCategories = Object, BlueprintType)
class HEXAGONS_API URuntimeMeshProviderHexagons : public URuntimeMeshProvider
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHexObstacle> ObstaclesToRender;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor ObstacleColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor CoreColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloorLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloorDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor FloorColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Material;


	virtual IRuntimeMeshProviderProxyRef GetProxy() override { return MakeShared<FRuntimeMeshProviderHexagonsProxy, ESPMode::ThreadSafe>(TWeakObjectPtr<URuntimeMeshProvider>(this), Material,
		ObstaclesToRender, ObstacleColor, Sides, CoreLength, CoreColor, FloorLength, FloorDistance, FloorColor, UKismetSystemLibrary::GetGameTimeInSeconds(this)); }
};
