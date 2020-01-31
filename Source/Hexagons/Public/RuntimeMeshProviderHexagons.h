// Copyright 2016-2019 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "Structs.h"
#include "RuntimeMeshProviderHexagons.generated.h"

class HEXAGONS_API FRuntimeMeshProviderHexagonsProxy : public FRuntimeMeshProviderProxy
{
	FHexRenderData RenderData;

	float RenderTime;

private:
	int32 AddVertex(FRuntimeMeshRenderableMeshData& MeshData, FVector location, FColor color);

	int32 AddVertexCollision(FRuntimeMeshCollisionData & CollisionData, FVector location);

public:
	FRuntimeMeshProviderHexagonsProxy(TWeakObjectPtr<URuntimeMeshProvider> InParent);
	~FRuntimeMeshProviderHexagonsProxy();

	void UpdateProxyParameters(URuntimeMeshProvider* ParentProvider, bool bIsInitialSetup) override;

	virtual void Initialize() override;

	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;

	FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	bool HasCollisionMesh() override;
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData) override;

	virtual FBoxSphereBounds GetBounds() override { return FBoxSphereBounds(FSphere(FVector::ZeroVector, RenderData.FloorLength)); }

	bool IsThreadSafe() const override;

};

UCLASS(HideCategories = Object, BlueprintType)
class HEXAGONS_API URuntimeMeshProviderHexagons : public URuntimeMeshProvider
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FHexRenderData RenderData;


	virtual FRuntimeMeshProviderProxyRef GetProxy() override { return MakeShared<FRuntimeMeshProviderHexagonsProxy, ESPMode::ThreadSafe>(TWeakObjectPtr<URuntimeMeshProvider>(this)); }
};
