// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Unita.generated.h"

// Forward declaration
class AGameManager;

UENUM(BlueprintType)
enum class ETipoAttacco : uint8
{
    CortoRaggio    UMETA(DisplayName = "Corto Raggio"),
    Distanza       UMETA(DisplayName = "Distanza")
};

UENUM(BlueprintType)
enum class ETipoUnita : uint8
{
    Sniper         UMETA(DisplayName = "Sniper"),
    Brawler        UMETA(DisplayName = "Brawler")
};

UCLASS()
class GAME_API AUnita : public AActor
{
    GENERATED_BODY()

public:
    AUnita();

    // Riferimento al GameManager per le callback
    UPROPERTY()
    class AGameManager* GameManager;

    // Parametri dell'unita
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    ETipoUnita TipoUnita = ETipoUnita::Sniper;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 Movimento = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    ETipoAttacco TipoAttacco = ETipoAttacco::CortoRaggio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 RangeAttacco = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 DannoMin = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 DannoMax = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 PuntiVita = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    int32 PuntiVitaMax = 10;

    // Posizione sulla griglia
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unita")
    int32 PosX = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unita")
    int32 PosY = 0;

    // Controllo (true = giocatore, false = IA)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unita")
    bool bIsGiocatore = true;

    // Colore dell'unita
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unita")
    FColor ColoreUnita = FColor::Cyan;

    // ===== VARIABILI PER MOVIMENTO ANIMATO =====
    UPROPERTY()
    bool bIsBeingDestroyed = false;

    UPROPERTY()
    TArray<FIntPoint> PercorsoMovimento;  // Celle del percorso da seguire

    UPROPERTY()
    int32 IndicePercorsoCorrente;  // Indice della cella corrente nel percorso

    UPROPERTY()
    bool bSiStaMuovendo;  // True se l'unità sta eseguendo un movimento animato

    UPROPERTY()
    float TempoTrascorsoPerCella;  // Tempo trascorso nella cella corrente

    UPROPERTY()
    float TempoPerCella = 0.3f;  // Secondi per spostarsi da una cella all'altra
    // ===== FINE VARIABILI MOVIMENTO ANIMATO =====

    // Flag per il turno
    UPROPERTY()
    bool bHaMosso = false;

    UPROPERTY()
    bool bHaAttaccato = false;

    UPROPERTY()
    bool bHaAgitoNelTurno = false;

    // Funzioni pubbliche
    UFUNCTION(BlueprintCallable, Category = "Unita")
    int32 CalcolaDanno() const;

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void PrendiDanno(int32 DannoSubito);

    UFUNCTION(BlueprintCallable, Category = "Unita")
    bool IsVivo() const;

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void SetPosizione(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Unita")
    FString GetTipoAttaccoAsString() const;

    UFUNCTION(BlueprintCallable, Category = "Unita")
    FString GetTipoUnitaAsString() const;

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void ResettaVita();

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void ImpostaDannoPerTipo();

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void DisegnaUnita();

    UFUNCTION(BlueprintCallable, Category = "Unita")
    void ResetTurno();

    // ===== FUNZIONI PER MOVIMENTO ANIMATO =====
    UFUNCTION()
    void IniziaMovimentoAnimato(const TArray<FIntPoint>& Percorso);

    UFUNCTION()
    void AggiornaMovimento(float DeltaTime);

    UFUNCTION()
    void ProssimaCella();
    // ===== FINE FUNZIONI MOVIMENTO ANIMATO =====

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};