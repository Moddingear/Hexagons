// Fill out your copyright notice in the Description page of Project Settings.


#include "HexRenderingComponent.h"
#include "UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

UHexRenderingComponent::UHexRenderingComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/MeshMaterial.MeshMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		MeshMaterial = MaterialFinder.Object;
	}
	bReplicates = true;
	ObstacleColor = FColor(247, 243, 0);
	Sides = 6.f;
	CoreLength = 100.f;
	CoreColor = FColor(247, 57, 0);
	FloorLength = 10000.f;
	FloorDistance = -10.f;
	FloorColor = FColor(247, 0, 0);
}

void UHexRenderingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHexRenderingComponent, ObstaclesToRender);
}

void UHexRenderingComponent::UpdateMesh()
{
	TArray<int32> ObstaclesToDelete;
	for (int32 i = 0; i < ObstaclesToRender.Num(); i++)
	{
		FHexObstacle Obstacle = ObstaclesToRender[i];
		float Distance = Obstacle.GetDistance(this);
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar < CoreLength) //If obstacle is inside the core, don't render
		{
			ObstaclesToDelete.Add(i);
			continue;
		}
	}

	for (int32 i = 0; i < ObstaclesToDelete.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Deleted obstacle %i"), ObstaclesToDelete.Last(i));
		ObstaclesToRender.RemoveAt(ObstaclesToDelete.Last(i));
	}

	//Ideally the provider shouldn't be recreated every tick
	URuntimeMeshProviderHexagons* Provider = NewObject<URuntimeMeshProviderHexagons>(this);
	Provider->ObstaclesToRender = ObstaclesToRender;
	Provider->ObstacleColor = ObstacleColor;
	Provider->Sides = Sides;
	Provider->CoreLength = CoreLength;
	Provider->CoreColor = CoreColor;
	Provider->FloorLength = FloorLength;
	Provider->FloorDistance = FloorDistance;
	Provider->FloorColor = FloorColor;

	Provider->Material = MeshMaterial;

	GetOrCreateRuntimeMesh()->Initialize(Provider);
}
