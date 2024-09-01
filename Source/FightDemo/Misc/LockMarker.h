// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LockMarker.generated.h"

UCLASS()
class FIGHTDEMO_API ALockMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALockMarker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mark, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* mark;

public:	
	void Attach(TWeakObjectPtr<AActor> target);
};
