#include "MyPlayerController.h"
#include "GameManager.h"
#include "MyActor.h"
#include "WBP_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMyPlayerController::AMyPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    DefaultMouseCursor = EMouseCursor::Crosshairs;

    // Carica HUD
    static ConstructorHelpers::FObjectFinder<UClass> WidgetClassFinder(TEXT("/Game/WBP_HUD.WBP_HUD_C"));
    if (WidgetClassFinder.Succeeded())
    {
        WidgetHUDClass = WidgetClassFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("Widget HUD Class caricata automaticamente dal C++!"));
    }
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("=== MyPlayerController avviato ==="));
    UE_LOG(LogTemp, Warning, TEXT("Clicca sulla griglia per interagire."));

    // Trova GameManager
    GameManager = Cast<AGameManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

    // Trova MyActor
    GridActor = Cast<AMyActor>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMyActor::StaticClass()));

    if (GridActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridActor TROVATO! GridSize=%d, CellSize=%.1f"),
            GridActor->GridSize, GridActor->CellSize);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GridActor NON TROVATO!"));
    }

    // Crea HUD
    if (WidgetHUDClass && GameManager)
    {
        UWBP_HUD* HUD = CreateWidget<UWBP_HUD>(this, WidgetHUDClass);
        if (HUD)
        {
            HUD->AddToViewport();
            HUD->GameManagerRef = GameManager;
            GameManager->WidgetHUD = HUD;
            HUD->AggiornaHUD();
            UE_LOG(LogTemp, Warning, TEXT("HUD creato e collegato al GameManager."));
        }
    }
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMyPlayerController::OnClickSinistro);
}

void AMyPlayerController::OnClickSinistro()
{
    if (!GameManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GameManager NULL, click ignorato"));
        return;
    }

    // Ottieni la posizione del mouse nel mondo
    FVector WorldLocation, WorldDirection;
    if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
    {
        UE_LOG(LogTemp, Warning, TEXT("Impossibile ottenere posizione mouse"));
        return;
    }

    // Calcola intersezione con il piano Z = 0 (piano della griglia)
    if (FMath::Abs(WorldDirection.Z) > KINDA_SMALL_NUMBER)
    {
        float t = -WorldLocation.Z / WorldDirection.Z;
        if (t > 0)
        {
            FVector HitLocation = WorldLocation + WorldDirection * t;

            int32 GridX, GridY;
            if (ConvertHitInCoordGriglia(HitLocation, GridX, GridY))
            {
                UE_LOG(LogTemp, Warning, TEXT("Click su cella (%d, %d)"), GridX, GridY);
                GameManager->GestisciClickCella(GridX, GridY);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Click fuori dalla griglia"));
            }
        }
    }
}

bool AMyPlayerController::ConvertHitInCoordGriglia(const FVector& HitLocation, int32& OutX, int32& OutY)
{
    if (!GridActor) return false;

    float MetaGrid = GridActor->GridSize / 2.0f;
    OutX = FMath::FloorToInt(HitLocation.X / GridActor->CellSize + MetaGrid);
    OutY = FMath::FloorToInt(HitLocation.Y / GridActor->CellSize + MetaGrid);

    if (OutX < 0 || OutX >= GridActor->GridSize || OutY < 0 || OutY >= GridActor->GridSize)
    {
        return false;
    }

    return true;
}