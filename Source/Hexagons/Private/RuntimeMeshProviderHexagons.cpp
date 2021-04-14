// Copyright 2019-2021 Gabriel Zerbib (Moddingear). All Rights Reserved.


#include "RuntimeMeshProviderHexagons.h"
#include "..\Public\RuntimeMeshProviderHexagons.h"

void URuntimeMeshProviderHexagons::SetRenderData(FHexRenderData & InRenderData)
{
	FScopeLock Lock(&PropertySyncRoot);
	RenderData = InRenderData;
	MarkAllLODsDirty();
	MarkCollisionDirty();
}

FHexRenderData URuntimeMeshProviderHexagons::GetRenderData()
{
	FScopeLock Lock(&PropertySyncRoot);
	return RenderData;
}

int32 URuntimeMeshProviderHexagons::AddVertex(FRuntimeMeshRenderableMeshData & MeshData, FVector location, FColor color)
{
	int32 vertindex = MeshData.Positions.Add(location);
	MeshData.Tangents.Add(FVector(0, 0, 1), FVector(1, 0, 0));
	MeshData.TexCoords.Add(FVector2D(0, 0));
	MeshData.Colors.Add(color);
	return vertindex;
}

int32 URuntimeMeshProviderHexagons::AddVertexCollision(FRuntimeMeshCollisionData & CollisionData, FVector location)
{
	int32 vertindex = CollisionData.Vertices.Add(location);
	return vertindex;
}

void URuntimeMeshProviderHexagons::Initialize()
{
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	TArray<FRuntimeMeshLODProperties> LODs;
	LODs.Add(LODProperties);
	ConfigureLODs(LODs);

	SetupMaterialSlot(0, FName("Material"), RenderData.Material);

	FRuntimeMeshSectionProperties Properties;
	Properties.bCastsShadow = true;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Frequent;
	CreateSection(0, 0, Properties);
}

FBoxSphereBounds URuntimeMeshProviderHexagons::GetBounds()
{

	return FBoxSphereBounds(FSphere(FVector::ZeroVector, RenderData.FloorLength));
}

bool URuntimeMeshProviderHexagons::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_ProviderHexagon_GetSectionMesh );
	FDateTime RenderTime = FDateTime::UtcNow();
	FHexRenderData TempRenderData;
	{
		FScopeLock Lock(&PropertySyncRoot);
		TempRenderData = RenderData;
	}
	// We should only ever be queried for section 0 and lod 0
	check(SectionId == 0 && LODIndex == 0);

	//pre-allocate vertices and triangles so it doesn't need to shuffle data around (slightly faster)
	uint8 NumSides = FMath::CeilToInt(TempRenderData.Sides);
	int32 NumObstacles = TempRenderData.ObstaclesToRender.Num();
	int32 NumVertices = 2*3*static_cast<int32>(NumSides) + 8 * NumObstacles;
	int32 NumTriangles = 2*static_cast<int32>(NumSides) + 4 * NumObstacles;
	MeshData.ReserveVertices(NumVertices);
	MeshData.Triangles.Reserve(NumTriangles *3);
	
	//First step, pre-build the directions of all of the sides (cos and sine are expensive, better not compute them too much)
	TArray<FVector> Directions;
	for (uint8 side = 0; side < NumSides; side++)
	{
		float angle = (side / TempRenderData.Sides) * 2 * PI;
		if (angle > 2 * PI)
		{
			angle = 2 * PI;
		}
		float x, y;
		FMath::SinCos(&x, &y, angle);
		Directions.Emplace(x, y, 0.f);
	}

	//Build core and floor
	for (uint8 i = 0; i < NumSides; i++)
	{
		FVector left = Directions[i];
		FVector right = Directions[(i + 1) % Directions.Num()];
		FVector floor = FVector(0.f, 0.f, TempRenderData.FloorDistance);
		TArray<FVector> Locations = TArray<FVector>({ left, right, FVector::ZeroVector }); //Triangles going clockwise are visible
		for (uint8 j = 0; j < 3; j++) //Core triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * TempRenderData.CoreLength, TempRenderData.CoreColor);
			MeshData.Triangles.Add(index);
		}
		for (uint8 j = 0; j < 3; j++) //Floor triangle
		{
			int32 index = AddVertex(MeshData, Locations[j] * TempRenderData.FloorLength + floor, (i&1)>0 ? TempRenderData.FloorColorOdd : TempRenderData.FloorColorEven);
			MeshData.Triangles.Add(index);
		}
	}

	int32 numRendered = 0;
	//Build obstacles
	for (int32 i = 0; i < NumObstacles; i++)
	{
		FHexObstacle Obstacle = TempRenderData.ObstaclesToRender[i];

		if (Obstacle.Side >= Directions.Num())
		{
			UE_LOG(LogTemp, Log, TEXT("Obstacle not rendered because on wrong side : %s"), *Obstacle.ToString())
			continue;
		}
		FVector left = Directions[Obstacle.Side];
		FVector right = Directions[(Obstacle.Side + 1) % Directions.Num()];
		float Distance = Obstacle.GetDistance(RenderTime) + TempRenderData.CoreLength;
		//UE_LOG(LogTemp, Log, TEXT("Obstacle %i is at distance %f"), i, Distance);
		float DistanceFar = Distance + Obstacle.Thickness;

		float EdgeDistance = FMath::Lerp(DistanceFar, Distance, TempRenderData.EdgeProportion);
		
		if (DistanceFar < TempRenderData.CoreLength) //If obstacle is inside the core, don't render
		{
			continue;
		}
		Distance = FMath::Max(TempRenderData.CoreLength, Distance); //Clamp the close side distance so it doesn't clip the core
		EdgeDistance = FMath::Max(TempRenderData.CoreLength, EdgeDistance);
		
		FVector CloseLeft = left * Distance, CloseRight = right * Distance, FarLeft = left * DistanceFar, FarRight = right * DistanceFar;

		FVector EdgeLeft = left * EdgeDistance, EdgeRight = right * EdgeDistance;

		//The edge vertices need to be doubled so that one set can have the wall color and the other set can have the edge color
		TArray<FVector> Location = TArray<FVector>({ CloseLeft, CloseRight, EdgeLeft, EdgeRight, EdgeLeft, EdgeRight, FarLeft, FarRight });
		TArray<int32> Triangles = TArray<int32>({ 0,2,1, 1,2,3, 4,6,5, 5,6,7 });
		int32 VertOffset = MeshData.Positions.Num();
		for (uint8 j = 0; j < Location.Num(); j++)
		{
			AddVertex(MeshData, Location[j], j >= 4 ? TempRenderData.ObstacleEdgeColor : TempRenderData.ObstacleColor);
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

FRuntimeMeshCollisionSettings URuntimeMeshProviderHexagons::GetCollisionSettings()
{
	FRuntimeMeshCollisionSettings Settings;
	Settings.bUseAsyncCooking = true;
	Settings.bUseComplexAsSimple = false;
	
	return Settings;
}

bool URuntimeMeshProviderHexagons::HasCollisionMesh()
{
	return true;
}

bool URuntimeMeshProviderHexagons::GetCollisionMesh(FRuntimeMeshCollisionData& CollisionData)
{
	float RenderTime = UKismetSystemLibrary::GetGameTimeInSeconds(this);

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

bool URuntimeMeshProviderHexagons::IsThreadSafe()
{
	return true;
}
