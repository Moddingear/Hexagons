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
		RenderData.Material = MaterialFinder.Object;
	}
	bReplicates = true;
	RenderData.ObstacleColor = FColor(247, 243, 0);
	RenderData.Sides = 6.f;
	RenderData.CoreLength = 100.f;
	RenderData.CoreColor = FColor(247, 57, 0);
	RenderData.FloorLength = 10000.f;
	RenderData.FloorDistance = -10.f;
	RenderData.FloorColorEven = FColor(247, 0, 0);
	RenderData.FloorColorOdd = FColor(186, 0, 0);
}

void UHexRenderingComponent::BeginPlay()
{
	HexagonProvider = NewObject<URuntimeMeshProviderHexagons>(this);
	HexagonProvider->RenderData = RenderData;

	GetOrCreateRuntimeMesh()->Initialize(HexagonProvider);
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionProfileName("BlockAll");
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
		float Distance = Obstacle.GetDistance(this) + RenderData.CoreLength;
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar < RenderData.CoreLength) //If obstacle is inside the core, don't render
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
	RenderData.ObstaclesToRender = ObstaclesToRender;
	HexagonProvider->RenderData = RenderData;
	HexagonProvider->MarkProxyParametersDirty();
}
