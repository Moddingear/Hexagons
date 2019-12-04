// Copyright 2016-2019 Chris Conway (Koderz). All Rights Reserved.


#include "RuntimeMeshProviderHexagons.h"
#include "..\Public\RuntimeMeshProviderHexagons.h"

int32 FRuntimeMeshProviderHexagonsProxy::AddVertex(FRuntimeMeshRenderableMeshData & MeshData, FVector location, FColor color)
{
	int32 vertindex = MeshData.Positions.Add(location);
	MeshData.Tangents.Add(FVector(0, 0, 1), FVector(1, 0, 0));
	MeshData.TexCoords.Add(FVector2D(0, 0));
	MeshData.Colors.Add(color);
	return vertindex;
}

FRuntimeMeshProviderHexagonsProxy::FRuntimeMeshProviderHexagonsProxy(TWeakObjectPtr<URuntimeMeshProvider> InParent, UMaterialInterface* InMaterial,
	TArray<FHexObstacle> InObstaclesToRender, FColor InObstacleColor, float InSides, float InCoreLength, FColor InCoreColor, float InFloorLength, float InFloorDistance, FColor InFloorColor, float InRenderTime)
	: FRuntimeMeshProviderProxy(InParent), Material(InMaterial),
	ObstaclesToRender(InObstaclesToRender), ObstacleColor(InObstacleColor), Sides(InSides), CoreLength(InCoreLength), CoreColor(InCoreColor), FloorLength(InFloorLength), FloorDistance(InFloorDistance), FloorColor(InFloorColor), RenderTime(InRenderTime)
{

}

FRuntimeMeshProviderHexagonsProxy::~FRuntimeMeshProviderHexagonsProxy()
{

}

void FRuntimeMeshProviderHexagonsProxy::Initialize()
{
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	ConfigureLOD(0, LODProperties);

	SetupMaterialSlot(0, FName("Cube Base"), Material);

	FRuntimeMeshSectionProperties Properties;
	Properties.bCastsShadow = true;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	CreateSection(0, 0, Properties);
}

bool FRuntimeMeshProviderHexagonsProxy::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	// We should only ever be queried for section 0 and lod 0
	check(SectionId == 0 && LODIndex == 0);

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

	//Build core and floor
	for (uint8 i = 0; i < Directions.Num(); i++)
	{
		FVector left = Directions[i];
		FVector right = Directions[(i + 1) % Directions.Num()];
		FVector floor = FVector(0.f, 0.f, FloorDistance);
		TArray<FVector> Locations = TArray<FVector>({ left, right, FVector::ZeroVector }); //Triangles going clockwise are visible
		for (uint8 j = 0; j < 3; j++) //Core triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * CoreLength, CoreColor);
			MeshData.Triangles.Add(index);
		}
		for (uint8 j = 0; j < 3; j++) //Floor triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * FloorLength + floor, FloorColor);
			MeshData.Triangles.Add(index);
		}
	}

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
		float Distance = Obstacle.GetDistance(RenderTime);
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar < CoreLength) //If obstacle is inside the core, don't render
		{
			ObstaclesToDelete.Add(i);
			continue;
		}
		if (Distance < CoreLength) //Clamp obstacle to never clip inside of the core
		{
			Distance = CoreLength;
		}
		FVector CloseLeft = left * Distance, CloseRight = right * Distance, FarLeft = left * DistanceFar, FarRight = right * DistanceFar;
		TArray<FVector> Location = TArray<FVector>({ CloseLeft, CloseRight, FarLeft, FarRight });
		TArray<int32> Triangles = TArray<int32>({ 0,2,1, 1,2,3 });
		int32 VertOffset = MeshData.Positions.Num();
		for (uint8 j = 0; j < 4; j++)
		{
			AddVertex(MeshData, Location[j], ObstacleColor);
		}
		for (int32 index : Triangles)
		{
			MeshData.Triangles.Add(index + VertOffset);
		}
	}

	//Clear all invisible obstacles
	for (int32 i = 0; i < ObstaclesToDelete.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Deleted obstacle %i"), ObstaclesToDelete.Last(i));
		ObstaclesToRender.RemoveAt(ObstaclesToDelete.Last(i));
	}

	return true;
}

FRuntimeMeshCollisionSettings FRuntimeMeshProviderHexagonsProxy::GetCollisionSettings()
{
	FRuntimeMeshCollisionSettings Settings;
	Settings.bUseAsyncCooking = true;
	Settings.bUseComplexAsSimple = false;

	return Settings;
}

bool FRuntimeMeshProviderHexagonsProxy::HasCollisionMesh()
{
	return false;
}

bool FRuntimeMeshProviderHexagonsProxy::GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData)
{
	return false;
}

bool FRuntimeMeshProviderHexagonsProxy::IsThreadSafe() const
{
	return true;
}
