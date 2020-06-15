// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "LuaActor.h"
#include "IotComponentBase.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class IOT_API UIotComponentBase : public UStaticMeshComponent, public slua_Luabase, public ILuaTableObjectInterface {
	GENERATED_BODY()

	struct TickTmpArgs {
		float deltaTime;
		enum ELevelTick tickType;
		FActorComponentTickFunction* thisTickFunction;
	};
protected:
	virtual void BeginPlay() override {
		Super::BeginPlay();
		if (!init(this, "UIotComponentBase", LuaStateName, LuaFilePath))
			return;
		if (!GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
			ReceiveBeginPlay();
		PrimaryComponentTick.SetTickFunctionEnable(postInit("bCanEverTick"));
	}

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override {
		Super::EndPlay(EndPlayReason);
		if (!GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
			ReceiveEndPlay(EndPlayReason);
	}
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override {
		tickTmpArgs.deltaTime = DeltaTime;
		tickTmpArgs.tickType = TickType;
		tickTmpArgs.thisTickFunction = ThisTickFunction;
		if (!tickFunction.isValid()) {
			superTick();
			return;
		}
		tickFunction.call(luaSelfTable, DeltaTime);
	}
public:
	virtual void ProcessEvent(UFunction* func, void* params) override {
		if (luaImplemented(func, params))
			return;
		Super::ProcessEvent(func, params);
	}
	void superTick() override {
		Super::TickComponent(tickTmpArgs.deltaTime, tickTmpArgs.tickType, tickTmpArgs.thisTickFunction);
		if (!GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
			ReceiveTick(tickTmpArgs.deltaTime);
	}
	NS_SLUA::LuaVar getSelfTable() const {
		return luaSelfTable;
	}
public:
	UIotComponentBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: UStaticMeshComponent(ObjectInitializer)
	{
		PrimaryComponentTick.bCanEverTick = true;
	}
public:
	struct TickTmpArgs tickTmpArgs;
	// below UPROPERTY and UFUNCTION can't be put to macro LUABASE_BODY
	// so copy & paste them
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "slua")
		FString LuaFilePath;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "slua")
		FString LuaStateName;
	UFUNCTION(BlueprintCallable, Category = "slua")
	FLuaBPVar CallLuaMember(FString FunctionName, const TArray<FLuaBPVar>& Args) {
		return callMember(FunctionName, Args);
	}

};
