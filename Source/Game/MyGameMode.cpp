#include "MyGameMode.h"
#include "MyPlayerController.h"
#include "GameFramework/SpectatorPawn.h"

AMyGameMode::AMyGameMode()
{
    // Spettatore volante - camera semplice
    DefaultPawnClass = ASpectatorPawn::StaticClass();
    PlayerControllerClass = AMyPlayerController::StaticClass();

    UE_LOG(LogTemp, Warning, TEXT("MyGameMode creato - SpectatorPawn e MyPlayerController impostati"));
}