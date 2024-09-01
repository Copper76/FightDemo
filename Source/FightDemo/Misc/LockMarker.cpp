// Fill out your copyright notice in the Description page of Project Settings.


#include "LockMarker.h"

// Sets default values
ALockMarker::ALockMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	SetRootComponent(mark);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		// Set the static mesh to the sphere mesh asset
		mark->SetStaticMesh(SphereMeshAsset.Object);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load the default sphere mesh."));
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("Material'/Game/Materials/PM_Marker.PM_Marker'"));
	if (MaterialAsset.Succeeded())
	{
		mark->SetMaterial(0, MaterialAsset.Object);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load the default material."));
	}

	mark->SetRelativeScale3D(FVector(0.1f));
}

// Called when the game starts or when spawned
void ALockMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALockMarker::Attach(TWeakObjectPtr<AActor> target)
{
	if (target.IsValid())
	{
		SetActorLocation(target->GetActorLocation());
		AttachToActor(target.Get(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}

