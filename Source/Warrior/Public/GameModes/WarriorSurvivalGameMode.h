// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "TimerManager.h"
#include "WarriorSurvivalGameMode.generated.h"

class AWarriorEnemyCharacter;
class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EWarriorSurvivalGameModeState : uint8
{
	WaitSpawnNewWave,
	SpawningNewWave,
	InProgress,
	WaveCompleted,
	AllWavesDone,
	PlayerDied
};

USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn;

	UPROPERTY(EditAnywhere)
	int32 MinPerSpawnCount = 1;

	UPROPERTY(EditAnywhere)
	int32 MaxPerSpawnCount = 3;
};

USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FWarriorEnemyWaveSpawnerInfo> EnemyWaveSpawnerDefinitions;

	UPROPERTY(EditAnywhere)
	int32 TotalEnemyToSpawnThisWave = 1;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<USoundBase> WaveMusic;
};

USTRUCT(BlueprintType)
struct WARRIOR_API FWarriorSurvivalDifficultyWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWarriorGameDifficulty Difficulty = EWarriorGameDifficulty::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* EnemyWaveSpawnerDataTable = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalGameModeStateChangedDelegate, EWarriorSurvivalGameModeState, CurrentState);
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorSurvivalGameMode : public AWarriorBaseGameMode
{
	GENERATED_BODY()
	
public:
	AWarriorSurvivalGameMode();

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void StartSurvivalGame();

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void NotifyPlayerDied();

	UFUNCTION(BlueprintCallable, Category = "BattleMusic")
	void StopBattleMusic();

	UFUNCTION(BlueprintPure, Category = "Survival")
	FORCEINLINE bool HasSurvivalGameStarted() const { return bHasSurvivalGameStarted; }

	UFUNCTION(BlueprintPure, Category = "Survival")
	FORCEINLINE UDataTable* GetResolvedEnemyWaveSpawnerDataTable() const { return EnemyWaveSpawnerDataTable; }

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState InState);
	bool HasFinishedAllWaves() const;
	void ResolveEnemyWaveSpawnerDataTableForCurrentDifficulty();
	void PreLoadNextWaveEnemies();
	FWarriorEnemyWaveSpawnerTableRow* GetCurrentWaveSpawnerTableRow() const;
	int32 TrySpawnWaveEnemies();
	bool ShouldKeepSpawnEnemies() const;
	void InitializeBattleMusicAudioComponent();
	void TryUpdateBattleMusicForCurrentWave();
	void PlayBattleMusic(USoundBase* MusicToPlay, TSoftObjectPtr<USoundBase> MusicReference);
	void StartBattleMusic(USoundBase* MusicToPlay);

	UFUNCTION()
	void OnEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnBattleMusicFinished();

	UPROPERTY()
	EWarriorSurvivalGameModeState CurrentSurvivalGameModeState;

	UPROPERTY(BlueprintAssignable)
	FOnSurvivalGameModeStateChangedDelegate OnSurvivalGameModeStateChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	UDataTable* EnemyWaveSpawnerDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	TArray<FWarriorSurvivalDifficultyWaveConfig> DifficultyWaveConfigs;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	int32 TotalWavesToSpawn;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	int32 CurrentWaveCount = 1;

	UPROPERTY()
	int32 CurrentSpawnedEnemiesCounter = 0;

	UPROPERTY()
	int32 TotalSpawnedEnemiesThisWaveCounter = 0;

	UPROPERTY()
	TArray<AActor*> TargetPointsArray;

	UPROPERTY()
	float TimePassedSinceStart = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float SpawnNewWaveWaitTIme = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float SpawnEnemiesDelayTime = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float WaveCompletedWaitTime = 5.f;

	UPROPERTY()
	TMap< TSoftClassPtr<AWarriorEnemyCharacter>, UClass* > PreLoadedEnemyClassMap;

	UPROPERTY()
	UAudioComponent* BattleMusicAudioComponent = nullptr;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> CurrentBattleMusic;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> PendingBattleMusic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BattleMusic", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BattleMusicVolume = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BattleMusic", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BattleMusicFadeInTime = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BattleMusic", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BattleMusicFadeOutTime = 1.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	bool bHasSurvivalGameStarted = false;

	bool bHasSetSurvivalGameModeState = false;

	bool bIsSwitchingBattleMusic = false;

	FTimerHandle BattleMusicFadeTimerHandle;
};
