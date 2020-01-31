// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "Structs.h"
#include "RuntimeMeshProviderHexagons.h"
#include "HexRenderingComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class HEXAGONS_API UHexRenderingComponent : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:

	URuntimeMeshProviderHexagons* HexagonProvider;

	UHexRenderingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
		TArray<FHexObstacle> ObstaclesToRender;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FHexRenderData RenderData;

	UFUNCTION(BlueprintCallable)
	void UpdateMesh();
	
};
