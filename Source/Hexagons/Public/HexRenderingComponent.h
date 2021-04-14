// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "Structs.h"
#include "RuntimeMeshProviderHexagons.h"
#include "HexRenderingComponent.generated.h"

/**
 * This class is blueprint facing and handles the data, that is replication and removing walls that are no longer needed because they are past the center
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class HEXAGONS_API UHexRenderingComponent : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category="Utilities|Time", meta=(WorldContext="WorldContextObject") )
	static FDateTime GetGameStartTime(UObject* WorldContextObject);

	UPROPERTY(SkipSerialization)
	URuntimeMeshProviderHexagons* HexagonProvider;

	UHexRenderingComponent(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;

	UFUNCTION(NetMulticast, BlueprintCallable, Reliable)
	void AddWall(FHexObstacle ObstacleToAdd);

	UPROPERTY(Replicated, SkipSerialization)
	TArray<FHexObstacle> ObstaclesToRender; //List of walls, replicated only so that when late joining the clients get the already present walls

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SkipSerialization)
	FHexRenderData RenderData; //Cache for data to be rendered, that is then passed to the provider

	UFUNCTION(BlueprintCallable)
	void UpdateMesh(); //Function called when the source data is new and the mesh needs to be updated
	
};
