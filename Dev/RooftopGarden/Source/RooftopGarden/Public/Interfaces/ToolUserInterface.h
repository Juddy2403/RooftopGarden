// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                            #include "UObject/Interface.h"
#include "ToolUserInterface.generated.h"

class UAnimMontage;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UToolUserInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOFTOPGARDEN_API IToolUserInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ActionDone();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void StartAction();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SecondaryAction();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UAnimMontage* GetAnimationMontage();
	
};
