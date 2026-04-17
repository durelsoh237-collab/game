#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Unita.h"
#include "MyActor.generated.h"

// Enum per gli stati delle torri
UENUM(BlueprintType)
enum class EStatoTorre : uint8
{
    Neutrale      UMETA(DisplayName = "Neutrale"),
    ControlloP1   UMETA(DisplayName = "Controllo Giocatore"),
    ControlloIA   UMETA(DisplayName = "Controllo IA"),
    Contesa       UMETA(DisplayName = "Contesa")
};

// Struttura FCella
USTRUCT(BlueprintType)
struct FCella
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 Altezza = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsTower = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D Posizione = FVector2D::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Z = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 TowerID = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EStatoTorre StatoTorre = EStatoTorre::Neutrale;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 PuntiTorre = 0;

    FCella() {}
};

UCLASS()
class GAME_API AMyActor : public AActor
{
    GENERATED_BODY()

public:
    AMyActor();

    // ========== PARAMETRI CONFIGURABILI ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSize = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSize = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 MaxHeight = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float NoiseScale = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 NumTorri = 3;

    // ========== POSIZIONI TORRI ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreCentraleX = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreCentraleY = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreSinistraX = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreSinistraY = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreDestraX = 19;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Towers")
    int32 TorreDestraY = 12;

    // ========== DATI GRIGLIA ==========

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
    TArray<FCella> Griglia;

    // ========== HIGHLIGHT CELLE ==========

    UPROPERTY()
    TArray<FIntPoint> CelleMovimentoEvidenziate;

    UPROPERTY()
    TArray<FIntPoint> CelleAttaccoEvidenziate;

    UPROPERTY()
    FIntPoint CellaUnitaSelezionata = FIntPoint(-1, -1);

    UPROPERTY()
    bool bMostraHighlight = false;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void PulisciHighlight();

    // ========== FUNZIONI GRIGLIA (PUBBLICHE per menu configurazione) ==========

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GeneraGriglia();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void PosizionaTorri();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void PosizionaCamera();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    TArray<FIntPoint> GetCelleCalpestabili() const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsCellaCalpestabile(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    int32 GetAltezzaCella(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsTower(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetPosizioneMondo(int32 X, int32 Y) const;

    // ========== FUNZIONI MOVIMENTO ==========

    UFUNCTION(BlueprintCallable, Category = "Movement")
    int32 CalcolaCostoMovimento(int32 DaX, int32 DaY, int32 AX, int32 AY) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsMovimentoValido(int32 DaX, int32 DaY, int32 AX, int32 AY) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    TArray<FIntPoint> GetCelleRaggiungibili(int32 StartX, int32 StartY, int32 MovimentoMax) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    TArray<FIntPoint> GetPercorso(int32 DaX, int32 DaY, int32 AX, int32 AY, int32 MovimentoMax) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void DisegnaCelleRaggiungibili(const TArray<FIntPoint>& Celle);

    // ========== FUNZIONI ATTACCO ==========

    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 DistanzaManhattan(int32 X1, int32 Y1, int32 X2, int32 Y2) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<FIntPoint> GetLineaBresenham(int32 X1, int32 Y1, int32 X2, int32 Y2) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PuoAttaccare(int32 AttaccanteX, int32 AttaccanteY, int32 AttaccanteLivello,
        int32 DifensoreX, int32 DifensoreY, int32 DifensoreLivello,
        ETipoAttacco Tipo, int32 Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<FIntPoint> GetUnitaNelRaggio(int32 AttaccanteX, int32 AttaccanteY,
        ETipoAttacco Tipo, int32 Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool EseguiAttacco(class AUnita* Attaccante, class AUnita* Difensore);

    // ========== FUNZIONI SUPPORTO ==========

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool VerificaRaggiungibilita();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    FString GetCoordinateString(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GetCoordinateDaString(FString CoordString, int32& X, int32& Y) const;

    // ========== FUNZIONI DI ILLUMINAZIONE ==========

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreaLuceFissa();

    // ========== STATISTICHE ==========

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void MostraStatistiche();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    FIntPoint TrovaPosizioneTorreValida(FIntPoint PosizioneIdeale);
    bool IsCellaValidaPerTorre(int32 X, int32 Y) const;
    void DisegnaGriglia();
    FColor GetColoreCella(int32 Altezza, bool bIsTower, EStatoTorre Stato) const;
};