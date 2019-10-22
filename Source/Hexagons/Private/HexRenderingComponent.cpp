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
	ObstacleColor = FLinearColor(247.f / 255.f, 243.f / 255.f, 0.f);
	Sides = 6.f;
	CoreLength = 100.f;
	CoreColor = FLinearColor(247.f / 255.f, 57.f / 255.f, 0.f);
	FloorLength = 10000.f;
	FloorDistance = -10.f;
	FloorColor = FLinearColor(247.f / 255.f, 0.f, 0.f);
}

void UHexRenderingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHexRenderingComponent, ObstaclesToRender);
}

FRuntimeMeshDataStruct<FHexVertex, int32> UHexRenderingComponent::RenderMesh()
{
	FColor ObstacleFColor = ObstacleColor.ToFColor(false), CoreFColor = CoreColor.ToFColor(false), FloorFColor = FloorColor.ToFColor(false);
	//First step, pre-build the directions of all of the sides
	TArray<FVector> Directions;
	for (uint8 side = 0; side < FMath::CeilToInt(Sides); side++)
	{
		float angle = (side / Sides) * 2 * PI;
		if (angle > 2 * PI)
		{
			angle = 2 * PI;
		}
		float x, y;
		FMath::SinCos(&x, &y, angle);
		Directions.Emplace(x, y, 0.f);
	}
	//Create a mesh manager
	FRuntimeMeshManager<int32, FHexVertex, int32> MeshManager;

	//Build core and floor
	FHexVertex CoreCenter = FHexVertex(FVector(0.f, 0.f, 0.f), CoreFColor);
	FHexVertex FloorCenter = FHexVertex(FVector(0.f, 0.f, FloorDistance), FloorFColor);
	FRuntimeMeshDataStruct<FHexVertex, int32> coreMesh, floorMesh;
	for (uint8 i = 0; i < Directions.Num(); i++)
	{
		FVector left = Directions[i];
		FVector right = Directions[(i + 1) % Directions.Num()];
		FVector floor = FVector(0.f, 0.f, FloorDistance);
		TArray<FVector> Locations = TArray<FVector>({ left, right, FVector::ZeroVector }); //Triangles going clockwise are visible
		for (uint8 j = 0; j < 3; j++)
		{
			int32 index = coreMesh.Vertices.Emplace(Locations[j] * CoreLength, CoreFColor);
			coreMesh.Triangles.Add(index);
			index = floorMesh.Vertices.Emplace(Locations[j] * FloorLength + floor, FloorFColor);
			floorMesh.Triangles.Add(index);
		}
	}
	//Add to mesh manager
	MeshManager.AddMesh(-2, coreMesh);
	MeshManager.AddMesh(-1, floorMesh);

	//Build obstacles
	TArray<int32> ObstaclesToDelete;
	for (int32 i = 0; i < ObstaclesToRender.Num(); i++)
	{
		FHexObstacle Obstacle = ObstaclesToRender[i];

		if (Obstacle.Side >= Directions.Num())
		{
			continue;
		}
		FVector left = Directions[Obstacle.Side];
		FVector right = Directions[(Obstacle.Side + 1) % Directions.Num()];
		float Distance = Obstacle.GetDistance(this);
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar<CoreLength) //If obstacle is inside the core, don't render
		{
			if (GetWorld()->IsServer())
			{
				ObstaclesToDelete.Add(i);
			}
			continue;
		}
		if (Distance < CoreLength) //Clamp obstacle to never clip inside of the core
		{
			Distance = CoreLength;
		}
		FVector CloseLeft = left * Distance, CloseRight = right * Distance, FarLeft = left * DistanceFar, FarRight = right * DistanceFar;
		TArray<FVector> Location = TArray<FVector>({ CloseLeft, CloseRight, FarLeft, FarRight });
		TArray<FHexVertex> Vertices;
		TArray<int32> Triangles = TArray<int32>({ 0,2,1, 1,2,3 });
		for (uint8 j = 0; j < 4; j++)
		{
			Vertices.Emplace(Location[j], ObstacleFColor);
		}
		MeshManager.AddMesh(i - ObstaclesToDelete.Num(), FRuntimeMeshDataStruct<FHexVertex, int32>(Vertices, Triangles));
	}

	//Clear all invisible obstacles
	for (int32 i = 0; i < ObstaclesToDelete.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Deleted obstacle %i"), ObstaclesToDelete.Last(i));
		ObstaclesToRender.RemoveAt(ObstaclesToDelete.Last(i));
	}

	return MeshManager.GetMesh();
}

void UHexRenderingComponent::DisplayMesh()
{
	FRuntimeMeshDataStruct<FHexVertex, int32> mesh = RenderMesh();
	SetCollisionUseComplexAsSimple(true);
	SetCollisionProfileName("BlockAll");
	SetMeshSection(0, mesh.Vertices, mesh.Triangles, true, EUpdateFrequency::Frequent);
	SetMaterial(0, MeshMaterial);
}
