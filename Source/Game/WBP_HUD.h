#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyActor.h"
#include "WBP_HUD.generated.h"

class AGameManager;
class AUnita;
class UTextBlock;

UCLASS()
class GAME_API UWBP_HUD : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "HUD")
    AGameManager* GameManagerRef = nullptr;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void AggiornaHUD();

    // ===== WIDGET TURNO =====
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_NumeroTurno;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_TurnoCorrente;

    // ===== WIDGET GIOCATORE =====
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_SniperGiocatore;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_BrawlerGiocatore;

    // ===== WIDGET IA =====
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_SniperIA;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_BrawlerIA;

    // ===== WIDGET TORRI =====
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Torre1;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Torre2;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Torre3;
};