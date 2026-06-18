// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/WarriorSurvivalGameMode.h"
#include "Engine/AssetManager.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "WarriorFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

#include "WarriorDebugHelper.h"

AWarriorSurvivalGameMode::AWarriorSurvivalGameMode()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> EasyWaveTable(TEXT("/Game/GameModes/DD_EnemyWaveSpawner_Easy.DD_EnemyWaveSpawner_Easy"));
	static ConstructorHelpers::FObjectFinder<UDataTable> NormalWaveTable(TEXT("/Game/GameModes/DD_EnemyWaveSpawner_Normal.DD_EnemyWaveSpawner_Normal"));
	static ConstructorHelpers::FObjectFinder<UDataTable> HardWaveTable(TEXT("/Game/GameModes/DD_EnemyWaveSpawner_Hard.DD_EnemyWaveSpawner_Hard"));
	static ConstructorHelpers::FObjectFinder<UDataTable> VeryHardWaveTable(TEXT("/Game/GameModes/DD_EnemyWaveSpawner_VeryHard.DD_EnemyWaveSpawner_VeryHard"));

	if (DifficultyWaveConfigs.IsEmpty())
	{
		if (EasyWaveTable.Succeeded())
		{
			DifficultyWaveConfigs.Add({ EWarriorGameDifficulty::Easy, EasyWaveTable.Object });
		}
		if (NormalWaveTable.Succeeded())
		{
			DifficultyWaveConfigs.Add({ EWarriorGameDifficulty::Normal, NormalWaveTable.Object });
		}
		if (HardWaveTable.Succeeded())
		{
			DifficultyWaveConfigs.Add({ EWarriorGameDifficulty::Hard, HardWaveTable.Object });
		}
		if (VeryHardWaveTable.Succeeded())
		{
			DifficultyWaveConfigs.Add({ EWarriorGameDifficulty::VeryHard, VeryHardWaveTable.Object });
		}
	}
}

void AWarriorSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	EWarriorGameDifficulty SavedGameDifficulty;

	if (UWarriorFunctionLibrary::TryLoadSavedGameDifficulty(SavedGameDifficulty))
	{
		CurrentGameDifficulty = SavedGameDifficulty;
	}
}

void AWarriorSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	ResolveEnemyWaveSpawnerDataTableForCurrentDifficulty();

	checkf(EnemyWaveSpawnerDataTable,TEXT("Forgot to assign a valid data table in survival game mode blueprint"));

	TotalWavesToSpawn = EnemyWaveSpawnerDataTable->GetRowNames().Num();

	InitializeBattleMusicAudioComponent();
}

void AWarriorSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHasSurvivalGameStarted)
	{
		return;
	}

	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::WaitSpawnNewWave)
	{
		TimePassedSinceStart += DeltaTime;

		if (TimePassedSinceStart >= SpawnNewWaveWaitTIme)
		{
			TimePassedSinceStart = 0.f;

			SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::SpawningNewWave);
		}
	}

	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::SpawningNewWave)
	{
		TimePassedSinceStart += DeltaTime;

		if (TimePassedSinceStart >= SpawnEnemiesDelayTime)
		{
			const int32 SpawnedEnemiesThisTick = TrySpawnWaveEnemies();

			if (CurrentSurvivalGameModeState != EWarriorSurvivalGameModeState::SpawningNewWave)
			{
				TimePassedSinceStart = 0.f;
				return;
			}

			CurrentSpawnedEnemiesCounter += SpawnedEnemiesThisTick;

			TimePassedSinceStart = 0.f;

			SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::InProgress);
		}
	}

	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::WaveCompleted)
	{
		TimePassedSinceStart += DeltaTime;

		if (TimePassedSinceStart >= WaveCompletedWaitTime)
		{
			TimePassedSinceStart = 0.f;

			CurrentWaveCount++;

			if (HasFinishedAllWaves())
			{
				SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::AllWavesDone);
			}
			else
			{
				SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaitSpawnNewWave);
				PreLoadNextWaveEnemies();
			}
		}
	}
}

void AWarriorSurvivalGameMode::StartSurvivalGame()
{
	if (bHasSurvivalGameStarted)
	{
		return;
	}

	bHasSurvivalGameStarted = true;
	TimePassedSinceStart = 0.f;
	CurrentWaveCount = 1;
	CurrentSpawnedEnemiesCounter = 0;
	TotalSpawnedEnemiesThisWaveCounter = 0;

	SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaitSpawnNewWave);
	PreLoadNextWaveEnemies();
}

void AWarriorSurvivalGameMode::SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState InState)
{
	if (bHasSetSurvivalGameModeState && CurrentSurvivalGameModeState == InState)
	{
		return;
	}

	CurrentSurvivalGameModeState = InState;
	bHasSetSurvivalGameModeState = true;

	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::SpawningNewWave)
	{
		TryUpdateBattleMusicForCurrentWave();
	}
	else if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::PlayerDied ||
		CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::AllWavesDone)
	{
		StopBattleMusic();
	}

	OnSurvivalGameModeStateChanged.Broadcast(CurrentSurvivalGameModeState);
}

void AWarriorSurvivalGameMode::NotifyPlayerDied()
{
	if (!bHasSurvivalGameStarted ||
		CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::PlayerDied ||
		CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::AllWavesDone)
	{
		return;
	}

	SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::PlayerDied);
}

bool AWarriorSurvivalGameMode::HasFinishedAllWaves() const
{
	return CurrentWaveCount > TotalWavesToSpawn;
}

void AWarriorSurvivalGameMode::ResolveEnemyWaveSpawnerDataTableForCurrentDifficulty()
{
	for (const FWarriorSurvivalDifficultyWaveConfig& DifficultyWaveConfig : DifficultyWaveConfigs)
	{
		if (DifficultyWaveConfig.Difficulty == CurrentGameDifficulty && DifficultyWaveConfig.EnemyWaveSpawnerDataTable)
		{
			EnemyWaveSpawnerDataTable = DifficultyWaveConfig.EnemyWaveSpawnerDataTable;
			return;
		}
	}
}

void AWarriorSurvivalGameMode::PreLoadNextWaveEnemies()
{
	if (HasFinishedAllWaves())
	{
		return;
	}

	PreLoadedEnemyClassMap.Empty();

	const FWarriorEnemyWaveSpawnerTableRow* CurrentWaveSpawnerRow = GetCurrentWaveSpawnerTableRow();
	if (!CurrentWaveSpawnerRow)
	{
		return;
	}

	if (!CurrentWaveSpawnerRow->WaveMusic.IsNull())
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(CurrentWaveSpawnerRow->WaveMusic.ToSoftObjectPath());
	}

	for (const FWarriorEnemyWaveSpawnerInfo& SpawnerInfo : CurrentWaveSpawnerRow->EnemyWaveSpawnerDefinitions)
	{
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;

		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			SpawnerInfo.SoftEnemyClassToSpawn.ToSoftObjectPath(),
			FStreamableDelegate::CreateLambda(
				[SpawnerInfo, this]()
				{
					if (UClass* LoadedEnemyClass = SpawnerInfo.SoftEnemyClassToSpawn.Get())
					{
						PreLoadedEnemyClassMap.Emplace(SpawnerInfo.SoftEnemyClassToSpawn, LoadedEnemyClass);
					}
				}
			)
		);
	}
}

void AWarriorSurvivalGameMode::InitializeBattleMusicAudioComponent()
{
	if (BattleMusicAudioComponent)
	{
		return;
	}

	BattleMusicAudioComponent = NewObject<UAudioComponent>(this, TEXT("BattleMusicAudioComponent"));
	if (!BattleMusicAudioComponent)
	{
		return;
	}

	BattleMusicAudioComponent->bAutoActivate = false;
	BattleMusicAudioComponent->bAutoDestroy = false;
	BattleMusicAudioComponent->bIsUISound = true;
	BattleMusicAudioComponent->OnAudioFinished.AddUniqueDynamic(this, &ThisClass::OnBattleMusicFinished);
	BattleMusicAudioComponent->RegisterComponent();
}

void AWarriorSurvivalGameMode::TryUpdateBattleMusicForCurrentWave()
{
	if (HasFinishedAllWaves() || !EnemyWaveSpawnerDataTable)
	{
		return;
	}

	const FWarriorEnemyWaveSpawnerTableRow* CurrentWaveSpawnerRow = GetCurrentWaveSpawnerTableRow();
	if (!CurrentWaveSpawnerRow || CurrentWaveSpawnerRow->WaveMusic.IsNull())
	{
		return;
	}

	const TSoftObjectPtr<USoundBase> RequestedMusic = CurrentWaveSpawnerRow->WaveMusic;
	if (CurrentBattleMusic == RequestedMusic || PendingBattleMusic == RequestedMusic)
	{
		return;
	}

	PendingBattleMusic = RequestedMusic;

	TWeakObjectPtr<AWarriorSurvivalGameMode> WeakThis(this);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		RequestedMusic.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[WeakThis, RequestedMusic]()
			{
				if (!WeakThis.IsValid() || WeakThis->PendingBattleMusic != RequestedMusic)
				{
					return;
				}

				if (USoundBase* LoadedMusic = RequestedMusic.Get())
				{
					WeakThis->PlayBattleMusic(LoadedMusic, RequestedMusic);
					return;
				}

				WeakThis->PendingBattleMusic.Reset();
			}
		)
	);
}

void AWarriorSurvivalGameMode::PlayBattleMusic(USoundBase* MusicToPlay, TSoftObjectPtr<USoundBase> MusicReference)
{
	if (!MusicToPlay || CurrentBattleMusic == MusicReference)
	{
		PendingBattleMusic.Reset();
		return;
	}

	InitializeBattleMusicAudioComponent();

	if (!BattleMusicAudioComponent)
	{
		PendingBattleMusic.Reset();
		return;
	}

	GetWorldTimerManager().ClearTimer(BattleMusicFadeTimerHandle);

	CurrentBattleMusic = MusicReference;
	PendingBattleMusic.Reset();

	if (BattleMusicAudioComponent->IsPlaying() && BattleMusicFadeOutTime > 0.f)
	{
		bIsSwitchingBattleMusic = true;
		BattleMusicAudioComponent->FadeOut(BattleMusicFadeOutTime, 0.f);

		FTimerDelegate StartMusicDelegate;
		StartMusicDelegate.BindUObject(this, &ThisClass::StartBattleMusic, MusicToPlay);
		GetWorldTimerManager().SetTimer(BattleMusicFadeTimerHandle, StartMusicDelegate, BattleMusicFadeOutTime, false);
		return;
	}

	StartBattleMusic(MusicToPlay);
}

void AWarriorSurvivalGameMode::StartBattleMusic(USoundBase* MusicToPlay)
{
	if (!BattleMusicAudioComponent || !MusicToPlay)
	{
		return;
	}

	bIsSwitchingBattleMusic = false;

	BattleMusicAudioComponent->SetSound(MusicToPlay);
	BattleMusicAudioComponent->SetVolumeMultiplier(BattleMusicVolume);

	if (BattleMusicFadeInTime > 0.f)
	{
		BattleMusicAudioComponent->FadeIn(BattleMusicFadeInTime, BattleMusicVolume);
	}
	else
	{
		BattleMusicAudioComponent->Play();
	}
}

void AWarriorSurvivalGameMode::StopBattleMusic()
{
	GetWorldTimerManager().ClearTimer(BattleMusicFadeTimerHandle);

	PendingBattleMusic.Reset();
	CurrentBattleMusic.Reset();

	if (!BattleMusicAudioComponent || !BattleMusicAudioComponent->IsPlaying())
	{
		bIsSwitchingBattleMusic = false;
		return;
	}

	if (BattleMusicFadeOutTime > 0.f)
	{
		bIsSwitchingBattleMusic = false;
		BattleMusicAudioComponent->FadeOut(BattleMusicFadeOutTime, 0.f);
		return;
	}

	bIsSwitchingBattleMusic = false;
	BattleMusicAudioComponent->Stop();
}

void AWarriorSurvivalGameMode::OnBattleMusicFinished()
{
	if (bIsSwitchingBattleMusic || CurrentBattleMusic.IsNull())
	{
		return;
	}

	if (USoundBase* LoadedMusic = CurrentBattleMusic.Get())
	{
		StartBattleMusic(LoadedMusic);
		return;
	}

	const TSoftObjectPtr<USoundBase> MusicToRestart = CurrentBattleMusic;
	TWeakObjectPtr<AWarriorSurvivalGameMode> WeakThis(this);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		MusicToRestart.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[WeakThis, MusicToRestart]()
			{
				if (!WeakThis.IsValid() || WeakThis->CurrentBattleMusic != MusicToRestart)
				{
					return;
				}

				if (USoundBase* LoadedMusic = MusicToRestart.Get())
				{
					WeakThis->StartBattleMusic(LoadedMusic);
				}
			}
		)
	);
}

FWarriorEnemyWaveSpawnerTableRow* AWarriorSurvivalGameMode::GetCurrentWaveSpawnerTableRow() const
{
	if (!EnemyWaveSpawnerDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("No enemy wave spawner data table is assigned."));
		return nullptr;
	}

	const FName RowName = FName(TEXT("Wave") + FString::FromInt(CurrentWaveCount));

	FWarriorEnemyWaveSpawnerTableRow* FoundRow = EnemyWaveSpawnerDataTable->FindRow<FWarriorEnemyWaveSpawnerTableRow>(RowName, FString());

	if (!FoundRow)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Could not find a valid row under the name %s in the data table %s."),
			*RowName.ToString(),
			*EnemyWaveSpawnerDataTable->GetName()
		);
	}

	return FoundRow;

}

int32 AWarriorSurvivalGameMode::TrySpawnWaveEnemies()
{
	if (TargetPointsArray.IsEmpty())
	{
		UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(), TargetPointsArray);
	}
	checkf(!TargetPointsArray.IsEmpty(), TEXT("No valid target point found in level: %s for spawning enemies"), *GetWorld()->GetName());

	FWarriorEnemyWaveSpawnerTableRow* CurrentWaveSpawnerRow = GetCurrentWaveSpawnerTableRow();
	if (!CurrentWaveSpawnerRow)
	{
		SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::AllWavesDone);
		return 0;
	}

	uint32 EnemiesSpawnedThisTime = 0;

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FWarriorEnemyWaveSpawnerInfo& SpawnerInfo : CurrentWaveSpawnerRow->EnemyWaveSpawnerDefinitions)
	{
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;

		const int32 NumToSpawn = FMath::RandRange(SpawnerInfo.MinPerSpawnCount, SpawnerInfo.MaxPerSpawnCount);

		UClass* LoadedEnemyClass = PreLoadedEnemyClassMap.FindRef(SpawnerInfo.SoftEnemyClassToSpawn);
		if (!LoadedEnemyClass)
		{
			LoadedEnemyClass = SpawnerInfo.SoftEnemyClassToSpawn.LoadSynchronous();
			if (!LoadedEnemyClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to load enemy class for wave %d."), CurrentWaveCount);
				continue;
			}

			PreLoadedEnemyClassMap.Emplace(SpawnerInfo.SoftEnemyClassToSpawn, LoadedEnemyClass);
		}

		for (int32 i = 0; i < NumToSpawn; i++)
		{
			const int32 RandomTargetPointIndex = FMath::RandRange(0, TargetPointsArray.Num() - 1);
			const FVector SpawnOrigin = TargetPointsArray[RandomTargetPointIndex]->GetActorLocation();
			const FRotator SpawnRotation = TargetPointsArray[RandomTargetPointIndex]->GetActorForwardVector().ToOrientationRotator();

			FVector RandomLocation;
			UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(this, SpawnOrigin, RandomLocation, 400.f);

			RandomLocation += FVector(0.f, 0.f, 150.f);

			AWarriorEnemyCharacter* SpawnedEnemy = GetWorld()->SpawnActor<AWarriorEnemyCharacter>(LoadedEnemyClass, RandomLocation, SpawnRotation, SpawnParam);

			if (SpawnedEnemy)
			{
				SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnEnemyDestroyed);

				EnemiesSpawnedThisTime++;
				TotalSpawnedEnemiesThisWaveCounter++;
			}
			if (!ShouldKeepSpawnEnemies())
			{
				return EnemiesSpawnedThisTime;
			}

		}
	}
	return EnemiesSpawnedThisTime;
}

bool AWarriorSurvivalGameMode::ShouldKeepSpawnEnemies() const
{
	const FWarriorEnemyWaveSpawnerTableRow* CurrentWaveSpawnerRow = GetCurrentWaveSpawnerTableRow();
	return CurrentWaveSpawnerRow && TotalSpawnedEnemiesThisWaveCounter < CurrentWaveSpawnerRow->TotalEnemyToSpawnThisWave;
}

void AWarriorSurvivalGameMode::OnEnemyDestroyed(AActor* DestroyedActor)
{
	CurrentSpawnedEnemiesCounter--;

	Debug::Print(FString::Printf(TEXT("CurrentSpawnedEnemiesCounter:%i, TotalSpawnedEnemiesThisWaveCounter:%i"), CurrentSpawnedEnemiesCounter, TotalSpawnedEnemiesThisWaveCounter));

	if (ShouldKeepSpawnEnemies())
	{
		CurrentSpawnedEnemiesCounter += TrySpawnWaveEnemies();
	}
	else if (CurrentSpawnedEnemiesCounter == 0)
	{
		TotalSpawnedEnemiesThisWaveCounter = 0;
		CurrentSpawnedEnemiesCounter = 0;

		SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaveCompleted);
	}
}
