#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Unita.h"
#include "MyActor.h"
#include "WBP_HUD.h"
#include "GameManager.generated.h"

// Struct per gestire il respawn delle unità
USTRUCT()
struct FUnitaRespawnInfo
{
    GENERATED_BODY()

    UPROPERTY()
    bool bGiocatore;

    UPROPERTY()
    ETipoUnita Tipo;

    UPROPERTY()
    int32 X;

    UPROPERTY()
    int32 Y;

    FUnitaRespawnInfo() : bGiocatore(true), Tipo(ETipoUnita::Sniper), X(0), Y(0) {}

    FUnitaRespawnInfo(bool InGiocatore, ETipoUnita InTipo, int32 InX, int32 InY)
        : bGiocatore(InGiocatore), Tipo(InTipo), X(InX), Y(InY) {
    }
};

UENUM(BlueprintType)
enum class EFaseGioco : uint8
{
    LancioMoneta    UMETA(DisplayName = "Lancio Moneta"),
    Posizionamento  UMETA(DisplayName = "Posizionamento"),
    Gioco           UMETA(DisplayName = "Gioco"),
    Fine            UMETA(DisplayName = "Fine")
};

UCLASS()
class GAME_API AGameManager : public AActor
{
    GENERATED_BODY()

public:
    AGameManager();

    // Riferimenti
    UPROPERTY()
    AMyActor* GrigliaAttore;

    UPROPERTY()
    TArray<AUnita*> UnitaGiocatore;

    UPROPERTY()
    TArray<AUnita*> UnitaIA;

    // ========== RIFERIMENTO AL WIDGET HUD ==========
    UPROPERTY()
    UWBP_HUD* WidgetHUD;

    // Stato del gioco
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    EFaseGioco FaseCorrente = EFaseGioco::LancioMoneta;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    bool bTurnoGiocatore = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 TurnoCorrente = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    bool bVincitoreLancio = true;

    UPROPERTY()
    AUnita* UnitaSelezionata = nullptr;

    // Unità da posizionare
    UPROPERTY()
    TArray<ETipoUnita> UnitaDaPosizionareGiocatore;

    UPROPERTY()
    TArray<ETipoUnita> UnitaDaPosizionareIA;

    // Colori
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    FColor ColoreGiocatore = FColor::Cyan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    FColor ColoreIA = FColor::Magenta;

    // Zone di schieramento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    int32 ZonaSchieramentoGiocatoreMin = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    int32 ZonaSchieramentoGiocatoreMax = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    int32 ZonaSchieramentoIAMin = 22;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    int32 ZonaSchieramentoIAMax = 24;

    // Variabili turno
    UPROPERTY()
    TArray<FIntPoint> CelleEvidenziate;

    UPROPERTY()
    int32 UnitaAttiveNelTurno = 0;

    // Punteggio torri
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 PuntiGiocatore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 PuntiIA = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 PuntiGiocatoreTurnoPrecedente = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 PuntiIATurnoPrecedente = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 TurniControlloGiocatore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
    int32 TurniControlloIA = 0;

    // Storico e debug
    UPROPERTY()
    TArray<FString> StoricoMosse;

    UPROPERTY()
    TArray<FIntPoint> PosizioniInizialiGiocatore;

    UPROPERTY()
    TArray<FIntPoint> PosizioniInizialiIA;

    UPROPERTY()
    bool bMostraRangeMovimento = true;

    UPROPERTY()
    bool bFineTurnoInCorso = false;

    // ========== FUNZIONI PRINCIPALI ==========

    UFUNCTION(BlueprintCallable, Category = "Game")
    void InizializzaGioco();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void LancioMoneta();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void IniziaPosizionamento();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void PosizionaUnita(bool bGiocatore, ETipoUnita Tipo, int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void IniziaGioco();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void IniziaTurno();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void FineTurno();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void TurnoIA();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void SelezionaUnita(AUnita* Unita);

    UFUNCTION(BlueprintCallable, Category = "Game")
    TArray<FIntPoint> GetCelleRaggiungibiliPerUnitaSelezionata();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void MuoviUnitaSelezionata(int32 DestX, int32 DestY);

    UFUNCTION(BlueprintCallable, Category = "Game")
    TArray<AUnita*> GetUnitaNemicheNelRaggio(AUnita* Attaccante);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void AttaccaConUnitaSelezionata(AUnita* Bersaglio);

    UFUNCTION(BlueprintCallable, Category = "Game")
    bool IsCellaOccupata(int32 X, int32 Y) const;

    UFUNCTION(BlueprintCallable, Category = "Game")
    TArray<FIntPoint> GetCelleLiberePerPosizionamento() const;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ControllaVittoria();

    UFUNCTION(BlueprintCallable, Category = "Game")
    FColor GetColoreUnita(bool bGiocatore) const;

    // ========== FUNZIONI TORRI ==========

    UFUNCTION(BlueprintCallable, Category = "Game")
    void AggiornaStatoTorri();

    UFUNCTION(BlueprintCallable, Category = "Game")
    TArray<FIntPoint> GetCelleZonaCattura(int32 TorreX, int32 TorreY) const;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ControllaZonaTorre(int32 TorreX, int32 TorreY);

    UFUNCTION(BlueprintCallable, Category = "Game")
    FColor GetColoreTorre(EStatoTorre Stato) const;

    // ========== FUNZIONI UTILITÀ ==========

    UFUNCTION(BlueprintCallable, Category = "Game")
    void EvidenziaCella(int32 X, int32 Y, FColor Colore);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void PulisciEvidenziazioni();

    UFUNCTION(BlueprintCallable, Category = "Game")
    bool PuoMuoversiUnita(AUnita* Unita);

    UFUNCTION(BlueprintCallable, Category = "Game")
    bool PuoAttaccareUnita(AUnita* Unita);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ControllaFineTurno();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void NascondiRangeMovimento();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void AggiungiMossaAStorico(FString Mossa);

    UFUNCTION(BlueprintCallable, Category = "Game")
    FString GetStoricoMosse() const;

    UFUNCTION(BlueprintCallable, Category = "Game")
    int32 CalcolaDannoContrattacco(AUnita* Attaccante, AUnita* Difensore, int32 Distanza);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void RespawnUnitaMorte();

    UFUNCTION(BlueprintCallable, Category = "Game")
    bool ValutaPrioritaTorri(AUnita* Unita, FIntPoint& MiglioreTorre);

    UFUNCTION(BlueprintCallable, Category = "Game")
    float ValutaCellaIA(AUnita* Unita, int32 CX, int32 CY) const;

    // ========== INPUT GIOCATORE ==========

    UFUNCTION(BlueprintCallable, Category = "Input")
    void GestisciClickCella(int32 X, int32 Y);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
    int32 StatoInputGiocatore = 0;

    UPROPERTY()
    TArray<FIntPoint> CelleRangeMovimento;

    UPROPERTY()
    TArray<FIntPoint> CelleRangeAttacco;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void CreaUnitaIniziali();
    void CreaUnitaEffettiva(bool bGiocatore, ETipoUnita Tipo, int32 X, int32 Y);
    void PosizionamentoIA();
    void PassaProssimoTurnoPosizionamento();
    void MostraFaseCorrente();

    FTimerHandle TimerHandleIA;
};