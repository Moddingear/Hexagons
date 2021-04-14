// Fill out your copyright notice in the Description page of Project Settings.


#include "HexRenderingComponent.h"
#include "UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

FDateTime UHexRenderingComponent::GetGameStartTime(UObject* WorldContextObject)
{
	const float GameTime = UKismetSystemLibrary::GetGameTimeInSeconds(WorldContextObject);
	const FDateTime now = FDateTime::UtcNow();
	const FDateTime GameStart = now - FTimespan::FromSeconds(GameTime);
	return GameStart;
}

UHexRenderingComponent::UHexRenderingComponent(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/MeshMaterial.MeshMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		RenderData.Material = MaterialFinder.Object;
	}
	bReplicates=true;
}

void UHexRenderingComponent::OnRegister()
{
	Super::OnRegister();
	
	SetIsReplicated(true);
	RenderData.ObstacleColor = FColor(247, 57, 0);
	RenderData.ObstacleEdgeColor = FColor(237, 217,2);
	RenderData.Sides = 6.f;
	RenderData.CoreLength = 100.f;
	RenderData.CoreColor = FColor(247, 57, 0);
	RenderData.FloorLength = 10000.f;
	RenderData.FloorDistance = -10.f;
	RenderData.FloorColorEven = FColor(247, 0, 0);
	RenderData.FloorColorOdd = FColor(186, 0, 0);
	if (GetWorld()->IsServer())
	{
	}
}

void UHexRenderingComponent::BeginPlay()
{
	Super::BeginPlay();
	HexagonProvider = NewObject<URuntimeMeshProviderHexagons>(this);
	HexagonProvider->SetRenderData(RenderData);

	GetOrCreateRuntimeMesh()->Initialize(HexagonProvider);
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionProfileName("BlockAll");
}

void UHexRenderingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHexRenderingComponent, ObstaclesToRender, COND_InitialOnly);
}

void UHexRenderingComponent::AddWall_Implementation(FHexObstacle ObstacleToAdd)
{
	ObstaclesToRender.Add(ObstacleToAdd);
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
        //UE_LOG(LogTemp, Log, TEXT("Deleted obstacle %i"), ObstaclesToDelete.Last(i));
        ObstaclesToRender.RemoveAt(ObstaclesToDelete.Last(i));
    }
	RenderData.ObstaclesToRender = ObstaclesToRender;
	HexagonProvider->SetRenderData(RenderData);
}
