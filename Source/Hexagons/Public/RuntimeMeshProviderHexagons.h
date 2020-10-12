// Copyright 2016-2019 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "Structs.h"
#include "RuntimeMeshProviderHexagons.generated.h"

UCLASS(HideCategories = Object, BlueprintType)
class HEXAGONS_API URuntimeMeshProviderHexagons : public URuntimeMeshProvider
{
	GENERATED_BODY()

private:
	mutable FCriticalSection PropertySyncRoot;

	FHexRenderData RenderData;

public:
	UFUNCTION(BlueprintCallable)
	void SetRenderData(FHexRenderData &InRenderData);

	UFUNCTION(BlueprintPure)
	FHexRenderData GetRenderData();

private:
	int32 AddVertex(FRuntimeMeshRenderableMeshData& MeshData, FVector location, FColor color);

	int32 AddVertexCollision(FRuntimeMeshCollisionData & CollisionData, FVector location);

protected:
	void Initialize() override;
	FBoxSphereBounds GetBounds() override;
	bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;
	FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	bool HasCollisionMesh() override;
	bool GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData) override;
	bool IsThreadSafe() override;
};
