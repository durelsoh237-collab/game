#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WBP_HUD.h"
#include "MyPlayerController.generated.h"

UCLASS()
class GAME_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMyPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void OnClickSinistro();
    bool ConvertHitInCoordGriglia(const FVector& HitLocation, int32& OutX, int32& OutY);

    UPROPERTY()
    class AGameManager* GameManager;

    UPROPERTY()
    class AMyActor* GridActor;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<class UWBP_HUD> WidgetHUDClass;
};