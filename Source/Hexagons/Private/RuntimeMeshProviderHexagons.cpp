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

int32 FRuntimeMeshProviderHexagonsProxy::AddVertexCollision(FRuntimeMeshCollisionData & CollisionData, FVector location)
{
	int32 vertindex = CollisionData.Vertices.Add(location);
	return vertindex;
}

FRuntimeMeshProviderHexagonsProxy::FRuntimeMeshProviderHexagonsProxy(TWeakObjectPtr<URuntimeMeshProvider> InParent)
	:FRuntimeMeshProviderProxy(InParent)
{

}

FRuntimeMeshProviderHexagonsProxy::~FRuntimeMeshProviderHexagonsProxy()
{

}

void FRuntimeMeshProviderHexagonsProxy::UpdateProxyParameters(URuntimeMeshProvider * ParentProvider, bool bIsInitialSetup)
{
	URuntimeMeshProviderHexagons* HexagonsProvider = Cast<URuntimeMeshProviderHexagons>(ParentProvider);
	RenderData = HexagonsProvider->RenderData;


	RenderTime = UKismetSystemLibrary::GetGameTimeInSeconds(HexagonsProvider);
	MarkSectionDirty(INDEX_NONE, INDEX_NONE);
	MarkCollisionDirty();
	//UE_LOG(LogTemp, Log, TEXT("UpdateProxyParameters was called."))
}

void FRuntimeMeshProviderHexagonsProxy::Initialize()
{
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	ConfigureLOD(0, LODProperties);

	SetupMaterialSlot(0, FName("Material"), RenderData.Material);

	FRuntimeMeshSectionProperties Properties;
	Properties.bCastsShadow = true;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Frequent;
	CreateSection(0, 0, Properties);
}

bool FRuntimeMeshProviderHexagonsProxy::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	// We should only ever be queried for section 0 and lod 0
	check(SectionId == 0 && LODIndex == 0);

	//First step, pre-build the directions of all of the sides
	TArray<FVector> Directions;
	for (uint8 side = 0; side < FMath::CeilToInt(RenderData.Sides); side++)
	{
		float angle = (side / RenderData.Sides) * 2 * PI;
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
		FVector floor = FVector(0.f, 0.f, RenderData.FloorDistance);
		TArray<FVector> Locations = TArray<FVector>({ left, right, FVector::ZeroVector }); //Triangles going clockwise are visible
		for (uint8 j = 0; j < 3; j++) //Core triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * RenderData.CoreLength, RenderData.CoreColor);
			MeshData.Triangles.Add(index);
		}
		for (uint8 j = 0; j < 3; j++) //Floor triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * RenderData.FloorLength + floor, (i&1)>0 ? RenderData.FloorColorOdd : RenderData.FloorColorEven);
			MeshData.Triangles.Add(index);
		}
	}

	int32 numRendered = 0;
	//Build obstacles
	for (int32 i = 0; i < RenderData.ObstaclesToRender.Num(); i++)
	{
		FHexObstacle Obstacle = RenderData.ObstaclesToRender[i];

		if (Obstacle.Side >= Directions.Num())
		{
			UE_LOG(LogTemp, Log, TEXT("Obstacle not rendered because on wrong side : %s"), *Obstacle.ToString())
			continue;
		}
		FVector left = Directions[Obstacle.Side];
		FVector right = Directions[(Obstacle.Side + 1) % Directions.Num()];
		float Distance = Obstacle.GetDistance(RenderTime) + RenderData.CoreLength;
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar < RenderData.CoreLength) //If obstacle is inside the core, don't render
		{
			continue;
		}
		if (Distance < RenderData.CoreLength) //Clamp obstacle to never clip inside of the core
		{
			Distance = RenderData.CoreLength;
		}
		FVector CloseLeft = left * Distance, CloseRight = right * Distance, FarLeft = left * DistanceFar, FarRight = right * DistanceFar;
		TArray<FVector> Location = TArray<FVector>({ CloseLeft, CloseRight, FarLeft, FarRight });
		TArray<int32> Triangles = TArray<int32>({ 0,2,1, 1,2,3 });
		int32 VertOffset = MeshData.Positions.Num();
		for (uint8 j = 0; j < 4; j++)
		{
			AddVertex(MeshData, Location[j], RenderData.ObstacleColor);
		}
		for (int32 index : Triangles)
		{
			MeshData.Triangles.Add(index + VertOffset);
		}
		numRendered++;
	}
	//UE_LOG(LogTemp, Log, TEXT("Rendered %d sides out of %d total"), numRendered, ObstaclesToRender.Num())
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
	return true;
}

bool FRuntimeMeshProviderHexagonsProxy::GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData)
{
	//First step, pre-build the directions of all of the sides
	TArray<FVector> Directions;
	for (uint8 side = 0; side < FMath::CeilToInt(RenderData.Sides); side++)
	{
		float angle = (side / RenderData.Sides) * 2 * PI;
		if (angle > 2 * PI)
		{
			angle = 2 * PI;
		}
		float x, y;
		FMath::SinCos(&x, &y, angle);
		Directions.Emplace(x, y, 0.f);
	}

	//We only want obstacles in the collision mesh so that the player collides with them.
	//Build obstacles
	for (int32 i = 0; i < RenderData.ObstaclesToRender.Num(); i++)
	{
		FHexObstacle Obstacle = RenderData.ObstaclesToRender[i];

		if (Obstacle.Side >= Directions.Num())
		{
			continue;
		}
		FVector left = Directions[Obstacle.Side];
		FVector right = Directions[(Obstacle.Side + 1) % Directions.Num()];
		float Distance = Obstacle.GetDistance(RenderTime) + RenderData.CoreLength;
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;
		if (DistanceFar < RenderData.CoreLength) //If obstacle is inside the core, don't render
		{
			continue;
		}
		if (Distance < RenderData.CoreLength) //Clamp obstacle to never clip inside of the core
		{
			Distance = RenderData.CoreLength;
		}
		FVector CloseLeft = left * Distance, CloseRight = right * Distance, FarLeft = left * DistanceFar, FarRight = right * DistanceFar;
		TArray<FVector> Location = TArray<FVector>({ CloseLeft, CloseRight, FarLeft, FarRight });
		TArray<int32> Triangles = TArray<int32>({ 0,2,1, 1,2,3 });
		int32 VertOffset = CollisionData.Vertices.Num();
		for (uint8 j = 0; j < 4; j++)
		{
			AddVertexCollision(CollisionData, Location[j]);
		}
		CollisionData.Triangles.Add(VertOffset + Triangles[0], VertOffset + Triangles[1], VertOffset + Triangles[2]);
		CollisionData.Triangles.Add(VertOffset + Triangles[3], VertOffset + Triangles[4], VertOffset + Triangles[5]);
	}

	return true;
}

bool FRuntimeMeshProviderHexagonsProxy::IsThreadSafe() const
{
	return true;
}
