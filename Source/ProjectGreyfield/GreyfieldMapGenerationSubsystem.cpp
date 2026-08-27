// Operation Greyfield - see GreyfieldMapGenerationSubsystem.h for the design summary and the
// WITH_EDITOR known-gap note.

#include "GreyfieldMapGenerationSubsystem.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Math/RandomStream.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "EngineUtils.h"

namespace GreyfieldMapGen
{
	// 1 section/component, 63 quads/section - Unreal's own New Landscape default. Total
	// heightmap resolution per side is ComponentsPerSide * QuadsPerComponent + 1 verts.
	constexpr int32 QuadsPerComponent = 63;
	constexpr int32 NumSubsections = 1;

	// 1 unreal unit = 1cm convention (project default) -> 100uu/quad = 1m/quad.
	constexpr float LandscapeActorScaleXY = 100.f;
	constexpr float LandscapeActorScaleZ = 100.f;
}

ALandscape* UGreyfieldMapGenerationSubsystem::GenerateMapForWorld(UObject* WorldContextObject, EGreyfieldMapSize MapSize, int32 Seed)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Greyfield MapGen: GenerateMapForWorld - could not resolve a World from the given context object"));
		return nullptr;
	}
	UGreyfieldMapGenerationSubsystem* MapGen = World->GetSubsystem<UGreyfieldMapGenerationSubsystem>();
	if (!MapGen)
	{
		UE_LOG(LogTemp, Error, TEXT("Greyfield MapGen: GenerateMapForWorld - world has no UGreyfieldMapGenerationSubsystem"));
		return nullptr;
	}
	return MapGen->GenerateMap(MapSize, Seed);
}

FGreyfieldMapSizePreset UGreyfieldMapGenerationSubsystem::GetPresetForSize(EGreyfieldMapSize MapSize)
{
	FGreyfieldMapSizePreset Preset;
	Preset.SpawnRadiusFraction = 0.62f;

	switch (MapSize)
	{
	case EGreyfieldMapSize::Small2v2:
		Preset.ComponentsPerSide = 16;
		Preset.NumPlayers = 4;
		break;
	case EGreyfieldMapSize::Medium3v3:
		Preset.ComponentsPerSide = 32;
		Preset.NumPlayers = 6;
		break;
	case EGreyfieldMapSize::Large4v4:
		Preset.ComponentsPerSide = 64;
		Preset.NumPlayers = 8;
		break;
	case EGreyfieldMapSize::Gigantic:
		// Placeholder: same player count as Large, just far more terrain per player. Whether
		// Gigantic should also mean more player slots is an undecided design question, not
		// answered here - see PROJECT_STATUS.md.
		Preset.ComponentsPerSide = 128;
		Preset.NumPlayers = 8;
		break;
	}

	return Preset;
}

TArray<FGreyfieldPlayerSpawn> UGreyfieldMapGenerationSubsystem::BuildSpawnPoints(const FGreyfieldMapSizePreset& Preset, int32 SizeVerts, float QuadWorldSize) const
{
	TArray<FGreyfieldPlayerSpawn> Spawns;

	const int32 PerTeam = FMath::Max(1, Preset.NumPlayers / 2);
	const float MapHalfExtent = (SizeVerts - 1) * QuadWorldSize * 0.5f;
	const float SpawnRadius = MapHalfExtent * Preset.SpawnRadiusFraction;

	// Team A spreads across a 10..170 degree arc (the map's "north" half, avoiding the exact
	// east/west seam); Team B is each of those points rotated 180 degrees exactly, so the two
	// teams are always a perfect point-reflection of each other - not just similar, identical
	// under rotation once the heightmap mirrors the same way in BuildHeightmap.
	constexpr float ArcStartDeg = 10.f;
	constexpr float ArcEndDeg = 170.f;

	for (int32 i = 0; i < PerTeam; ++i)
	{
		const float Alpha = (PerTeam == 1) ? 0.5f : (float)i / (float)(PerTeam - 1);
		const float AngleRad = FMath::DegreesToRadians(FMath::Lerp(ArcStartDeg, ArcEndDeg, Alpha));
		const FVector TeamAPos(SpawnRadius * FMath::Cos(AngleRad), SpawnRadius * FMath::Sin(AngleRad), 0.f);
		const FVector TeamBPos = -TeamAPos;

		FGreyfieldPlayerSpawn A;
		A.Location = TeamAPos;
		A.PlayerIndex = Spawns.Num();
		A.TeamIndex = 0;
		Spawns.Add(A);

		FGreyfieldPlayerSpawn B;
		B.Location = TeamBPos;
		B.PlayerIndex = Spawns.Num();
		B.TeamIndex = 1;
		Spawns.Add(B);
	}

	return Spawns;
}

TArray<uint16> UGreyfieldMapGenerationSubsystem::BuildHeightmap(int32 SizeVerts, int32 Seed, const TArray<FGreyfieldPlayerSpawn>& Spawns, float QuadWorldSize, float ActorScaleZ) const
{
	TArray<uint16> Heights;
	Heights.SetNumUninitialized(SizeVerts * SizeVerts);

	const FRandomStream RNG(Seed);
	// PerlinNoise2D is a fixed deterministic function of its input coordinate - without a
	// per-seed offset every seed would sample the exact same noise field.
	const FVector2D SeedOffset(RNG.FRandRange(-10000.f, 10000.f), RNG.FRandRange(-10000.f, 10000.f));

	constexpr int32 NumOctaves = 5;
	constexpr float Persistence = 0.5f;
	constexpr float Lacunarity = 2.0f;
	// ~120 quads (~120m) per major hill cycle - large enough to read as rolling terrain at
	// RTS camera scale, not single-building-sized noise jitter.
	constexpr float BaseFrequency = 1.0f / 120.0f;

	constexpr float MaxWorldHeightCm = 3000.f;	// ~30m hills - hilly but fully buildable/traversable
	constexpr float MinWorldHeightCm = -600.f;	// ~6m shallow basins

	const int32 Center = (SizeVerts - 1) / 2;
	// Flat buildable pad radius under each spawn, scaled to map size (~5% of the side length).
	const float FlattenRadiusQuads = SizeVerts * 0.05f;

	// Spawns are stored relative to map center in world units; convert once to grid space.
	TArray<FVector2D> SpawnGridPositions;
	SpawnGridPositions.Reserve(Spawns.Num());
	for (const FGreyfieldPlayerSpawn& Spawn : Spawns)
	{
		SpawnGridPositions.Add(FVector2D(Spawn.Location.X / QuadWorldSize + Center, Spawn.Location.Y / QuadWorldSize + Center));
	}

	auto ComputeWorldHeightCm = [&](int32 X, int32 Y) -> float
	{
		float Noise = 0.f;
		float Amplitude = 1.f;
		float Frequency = BaseFrequency;
		float AmplitudeSum = 0.f;
		for (int32 Octave = 0; Octave < NumOctaves; ++Octave)
		{
			const FVector2D SamplePos = FVector2D((float)X, (float)Y) * Frequency + SeedOffset;
			Noise += FMath::PerlinNoise2D(SamplePos) * Amplitude;
			AmplitudeSum += Amplitude;
			Amplitude *= Persistence;
			Frequency *= Lacunarity;
		}
		Noise = AmplitudeSum > 0.f ? Noise / AmplitudeSum : 0.f;

		// Sign-preserving power curve: pushes mid-range noise toward zero so most of the map
		// reads as buildable plains, letting only the stronger peaks/troughs rise into real
		// hills or basins.
		const float Shaped = FMath::Clamp(FMath::Sign(Noise) * FMath::Pow(FMath::Abs(Noise), 1.6f), -1.f, 1.f);
		float WorldHeightCm = Shaped * (Shaped >= 0.f ? MaxWorldHeightCm : FMath::Abs(MinWorldHeightCm));

		// Flatten toward the plain reference height (0) near any spawn's buildable pad. Uses
		// the full (both-team) spawn list, so the resulting mask is itself point-symmetric -
		// safe to compute once per point and mirror below rather than recomputing per side.
		float NearestSpawnDistQuads = TNumericLimits<float>::Max();
		for (const FVector2D& SpawnGridPos : SpawnGridPositions)
		{
			NearestSpawnDistQuads = FMath::Min(NearestSpawnDistQuads, FVector2D::Distance(FVector2D((float)X, (float)Y), SpawnGridPos));
		}
		if (NearestSpawnDistQuads < FlattenRadiusQuads)
		{
			const float FalloffFactor = FMath::SmoothStep(0.f, 1.f, NearestSpawnDistQuads / FlattenRadiusQuads);
			WorldHeightCm = FMath::Lerp(0.f, WorldHeightCm, FalloffFactor);
		}

		return WorldHeightCm;
	};

	// Only evaluate the "top" half (y 0..Center) and mirror each result into the point-reflected
	// cell - guarantees exact 180-degree symmetry rather than merely similar statistics between
	// the two halves.
	for (int32 Y = 0; Y <= Center; ++Y)
	{
		for (int32 X = 0; X < SizeVerts; ++X)
		{
			const float WorldHeightCm = ComputeWorldHeightCm(X, Y);
			const float RawFloat = LandscapeDataAccess::MidValue + WorldHeightCm / (LANDSCAPE_ZSCALE * ActorScaleZ);
			const uint16 RawHeight = (uint16)FMath::Clamp(FMath::RoundToInt(RawFloat), 0, 65535);

			Heights[Y * SizeVerts + X] = RawHeight;
			Heights[(SizeVerts - 1 - Y) * SizeVerts + (SizeVerts - 1 - X)] = RawHeight;
		}
	}

	return Heights;
}

ALandscape* UGreyfieldMapGenerationSubsystem::GenerateMap(EGreyfieldMapSize MapSize, int32 Seed)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

#if !WITH_EDITOR
	UE_LOG(LogTemp, Error, TEXT("Greyfield MapGen: runtime landscape generation needs an Editor-context executable - ALandscapeProxy::Import() is compiled out of Shipping/packaged builds. See PROJECT_STATUS.md known gaps."));
	return nullptr;
#else
	const FGreyfieldMapSizePreset Preset = GetPresetForSize(MapSize);
	if (Seed == 0)
	{
		Seed = (int32)FMath::Rand();
	}

	const int32 SizeVerts = Preset.ComponentsPerSide * GreyfieldMapGen::QuadsPerComponent + 1;
	const float QuadWorldSize = GreyfieldMapGen::LandscapeActorScaleXY;

	LastGeneratedSpawns = BuildSpawnPoints(Preset, SizeVerts, QuadWorldSize);
	TArray<uint16> HeightData = BuildHeightmap(SizeVerts, Seed, LastGeneratedSpawns, QuadWorldSize, GreyfieldMapGen::LandscapeActorScaleZ);

	// Center the landscape on the world origin so the spawn locations (already computed
	// relative to map center) line up directly with world space.
	const float HalfExtent = (float)(SizeVerts - 1) * QuadWorldSize * 0.5f;
	const FVector LandscapeOrigin(-HalfExtent, -HalfExtent, 0.f);

	ALandscape* Landscape = World->SpawnActor<ALandscape>(LandscapeOrigin, FRotator::ZeroRotator);
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Greyfield MapGen: failed to spawn ALandscape"));
		return nullptr;
	}

	// MTL_MWAM_AutoMaterial_MASTER is a template meant to be turned into a Material Instance with
	// real textures plugged into its parameters - applied directly to a landscape (as this used
	// to do) its default parameter values are the plugin's own literal placeholder texture,
	// reading "ADD COLOR TEXTURE TO MATERIAL" (confirmed visually 2026-08-25, Unlit view). Use one
	// of the plugin's ready-made instances instead, which already have real textures wired up.
	// MountainRangeExample fits this generator's rolling-hills output best of the three; a
	// dedicated Greyfield-specific instance (own parameter tuning, not tied to a demo map) is
	// follow-up work, not done here.
	// 2026-08-26 diagnostic note: temporarily swapped this to a trivial engine material
	// (/Engine/BasicShapes/BasicShapeMaterial) to bisect the still-open Lit-mode-black bug (root
	// cause #4, PROJECT_STATUS.md) - it was STILL solid black in Lit mode with that material too,
	// conclusively ruling out the MW Auto Material as the cause. Reverted back to it here since
	// the real material isn't the problem either way.
	static const FSoftObjectPath AutoMaterialPath(TEXT("/Game/MWLandscapeAutoMaterial/Materials/Landscape/MTL_MWAM_Landscape_MountainRangeExample.MTL_MWAM_Landscape_MountainRangeExample"));
	if (UMaterialInterface* AutoMaterial = Cast<UMaterialInterface>(AutoMaterialPath.TryLoad()))
	{
		Landscape->LandscapeMaterial = AutoMaterial;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen: could not load MW Auto Material instance at %s - landscape will use the engine default material"), *AutoMaterialPath.ToString());
	}

	Landscape->SetActorRelativeScale3D(FVector(GreyfieldMapGen::LandscapeActorScaleXY, GreyfieldMapGen::LandscapeActorScaleXY, GreyfieldMapGen::LandscapeActorScaleZ));

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));

	// MW Auto Material blends purely from slope/height/world position - it needs no manually
	// painted landscape layers, so this stays empty.
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	MaterialLayerDataPerLayers.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

	Landscape->Import(FGuid::NewGuid(), 0, 0, SizeVerts - 1, SizeVerts - 1, GreyfieldMapGen::NumSubsections, GreyfieldMapGen::QuadsPerComponent,
		HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());

	Landscape->GetLandscapeInfo(); // registers the LandscapeInfo, matching the editor's own New Landscape flow

	// 2026-08-27, Lit-mode-black investigation (root cause #4, PROJECT_STATUS.md): UE 5.8 made Edit
	// Layers mandatory for every landscape (confirmed via the "Automatically enabling edit layers"
	// log line and the engine's own HasLayersContent deprecation warning: "Non-edit layer landscapes
	// are deprecated, all landscapes use the edit layer system now"). Edit Layers is a separate GPU
	// merge pass that produces the FINAL heightmap/normal/weightmap textures the renderer actually
	// samples - Import() writes CPU-side texture data directly (the old pre-Edit-Layers behavior,
	// kept for backward compatibility) and calls CreateDefaultLayer(), but never explicitly requests
	// that merge run. In the editor this gets triggered implicitly by normal tool interaction; a
	// landscape built purely in C++ at runtime, with no editor UI ever touching it, may never
	// receive that trigger - leaving the renderer sampling stale/default merged-layer data (a
	// degenerate/zero normal would explain every symptom seen: Lit and Lighting Only both solid
	// black regardless of GI/shadows/exposure/post-process/camera position/material, while Unlit -
	// which doesn't need a normal - works fine).
	//
	// RequestLayersContentUpdateForceAll() (tried first) only sets a queued flag consumed by
	// ULandscapeSubsystem::Tick on a later frame - confirmed by reading its implementation
	// (LandscapeEditLayers.cpp:6719, just does `LayerContentUpdateModes |= InModeMask`). That's
	// fine at PIE runtime (the world keeps ticking), but useless for baking a level at edit time in
	// a single script invocation with no guaranteed later tick. ForceUpdateLayersContent() (used
	// here instead) is the synchronous sibling actually used internally for exactly this situation -
	// UpdateLayersContent(bInWaitForStreaming=true, bFlushRender=true) - it merges and flushes
	// immediately, in this call, no waiting on ticks required.
	Landscape->ForceUpdateLayersContent();

	for (const FGreyfieldPlayerSpawn& Spawn : LastGeneratedSpawns)
	{
		if (APlayerStart* Start = World->SpawnActor<APlayerStart>(Spawn.Location + FVector(0.f, 0.f, 300.f), FRotator::ZeroRotator))
		{
			Start->PlayerStartTag = FName(*FString::Printf(TEXT("Team%d_Player%d"), Spawn.TeamIndex, Spawn.PlayerIndex));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen: generated %s map, seed=%d, %dx%d verts, %.0fx%.0fm, %d player starts"),
		*UEnum::GetValueAsString(MapSize), Seed, SizeVerts, SizeVerts,
		(SizeVerts - 1) * QuadWorldSize / 100.f, (SizeVerts - 1) * QuadWorldSize / 100.f, LastGeneratedSpawns.Num());

	// TEMP diagnostic (2026-08-26, Lit-mode-black investigation) - dump the actual runtime state of
	// every DirectionalLight in the world and this landscape's actual material/bounds right after
	// Import(), so we're checking real runtime values instead of assuming edit-time inspection of a
	// different level matches. Remove once root cause #4 is closed (see PROJECT_STATUS.md).
	{
		int32 LightCount = 0;
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			++LightCount;
			ADirectionalLight* Light = *It;
			UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Light->GetLightComponent());
			UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen DIAG: DirectionalLight '%s' intensity=%.2f mobility=%d visible=%d affectsWorld=%d castShadows=%d atmosphereSun=%d loc=%s"),
				*Light->GetName(),
				LightComp ? LightComp->Intensity : -1.f,
				LightComp ? (int32)LightComp->Mobility.GetValue() : -1,
				LightComp ? LightComp->IsVisible() : -1,
				LightComp ? LightComp->bAffectsWorld : -1,
				LightComp ? LightComp->CastShadows : -1,
				LightComp ? LightComp->bAtmosphereSunLight : -1,
				*Light->GetActorLocation().ToString());
		}
		UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen DIAG: total DirectionalLight actors in world = %d"), LightCount);

		UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen DIAG: Landscape actor='%s' material='%s' numComponents=%d bounds=%s mobility=%d"),
			*Landscape->GetName(),
			Landscape->LandscapeMaterial ? *Landscape->LandscapeMaterial->GetName() : TEXT("NULL"),
			Landscape->LandscapeComponents.Num(),
			*Landscape->GetComponentsBoundingBox().ToString(),
			(int32)Landscape->GetRootComponent()->Mobility.GetValue());

		if (Landscape->LandscapeComponents.Num() > 0 && Landscape->LandscapeComponents[0])
		{
			ULandscapeComponent* Comp0 = Landscape->LandscapeComponents[0];
			UE_LOG(LogTemp, Warning, TEXT("Greyfield MapGen DIAG: Component[0] castShadow=%d visible=%d weightmapLayers=%d lightingChannels(0/1/2)=%d/%d/%d"),
				Comp0->CastShadow, Comp0->IsVisible(), Comp0->GetWeightmapLayerAllocations().Num(),
				Comp0->LightingChannels.bChannel0, Comp0->LightingChannels.bChannel1, Comp0->LightingChannels.bChannel2);
		}
	}

	return Landscape;
#endif
}
