// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "Structs.h"
#include "RuntimeMeshManager.h"
#include "HexRenderingComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class HEXAGONS_API UHexRenderingComponent : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:

	UHexRenderingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* MeshMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	TArray<FHexObstacle> ObstaclesToRender;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	FLinearColor ObstacleColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Sides;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoreLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor CoreColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FloorLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FloorDistance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor FloorColor;

	FRuntimeMeshDataStruct<FHexVertex, int32> RenderMesh();

	UFUNCTION(BlueprintCallable)
	void DisplayMesh();
	
};
