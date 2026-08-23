// Operation Greyfield - harvestable resource deposit.

#include "GreyfieldResourceNode.h"
#include "UObject/ConstructorHelpers.h"

AGreyfieldResourceNode::AGreyfieldResourceNode()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;
	BodyMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	if (SphereMeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(SphereMeshFinder.Object);
	}
}

int32 AGreyfieldResourceNode::Harvest(int32 RequestedAmount)
{
	const int32 Taken = FMath::Clamp(RequestedAmount, 0, RemainingAmount);
	RemainingAmount -= Taken;

	if (RemainingAmount <= 0)
	{
		Destroy();
	}

	return Taken;
}
