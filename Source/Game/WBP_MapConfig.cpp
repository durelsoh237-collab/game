#include "WBP_MapConfig.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "MyActor.h"
#include "GameManager.h"

void UWBP_MapConfig::NativeConstruct()
{
    Super::NativeConstruct();

    // Trova il GameManager
    GameManager = Cast<AGameManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

    // Collega gli eventi dei bottoni
    if (GeneraMappaButton)
    {
        GeneraMappaButton->OnClicked.AddDynamic(this, &UWBP_MapConfig::OnGeneraMappaClicked);
    }

    if (MappaCasualeButton)
    {
        MappaCasualeButton->OnClicked.AddDynamic(this, &UWBP_MapConfig::OnMappaCasualeClicked);
    }

    // Carica i valori correnti se disponibili
    if (GameManager && GameManager->GrigliaAttore)
    {
        AMyActor* MyActor = GameManager->GrigliaAttore;

        if (GridSizeInput) GridSizeInput->SetText(FText::AsNumber(MyActor->GridSize));
        if (CellSizeInput) CellSizeInput->SetText(FText::AsNumber(MyActor->CellSize));
        if (MaxHeightInput) MaxHeightInput->SetText(FText::AsNumber(MyActor->MaxHeight));
        if (NumTorriInput) NumTorriInput->SetText(FText::AsNumber(MyActor->NumTorri));

        if (TorreCentraleXInput) TorreCentraleXInput->SetText(FText::AsNumber(MyActor->TorreCentraleX));
        if (TorreCentraleYInput) TorreCentraleYInput->SetText(FText::AsNumber(MyActor->TorreCentraleY));
        if (TorreSinistraXInput) TorreSinistraXInput->SetText(FText::AsNumber(MyActor->TorreSinistraX));
        if (TorreSinistraYInput) TorreSinistraYInput->SetText(FText::AsNumber(MyActor->TorreSinistraY));
        if (TorreDestraXInput) TorreDestraXInput->SetText(FText::AsNumber(MyActor->TorreDestraX));
        if (TorreDestraYInput) TorreDestraYInput->SetText(FText::AsNumber(MyActor->TorreDestraY));
    }
}

void UWBP_MapConfig::OnGeneraMappaClicked()
{
    if (!GameManager || !GameManager->GrigliaAttore) return;

    AMyActor* MyActor = GameManager->GrigliaAttore;

    // Leggi i valori dai campi di input
    if (GridSizeInput) MyActor->GridSize = FCString::Atoi(*GridSizeInput->GetText().ToString());
    if (CellSizeInput) MyActor->CellSize = FCString::Atof(*CellSizeInput->GetText().ToString());
    if (MaxHeightInput) MyActor->MaxHeight = FCString::Atoi(*MaxHeightInput->GetText().ToString());
    if (NumTorriInput) MyActor->NumTorri = FCString::Atoi(*NumTorriInput->GetText().ToString());

    if (TorreCentraleXInput) MyActor->TorreCentraleX = FCString::Atoi(*TorreCentraleXInput->GetText().ToString());
    if (TorreCentraleYInput) MyActor->TorreCentraleY = FCString::Atoi(*TorreCentraleYInput->GetText().ToString());
    if (TorreSinistraXInput) MyActor->TorreSinistraX = FCString::Atoi(*TorreSinistraXInput->GetText().ToString());
    if (TorreSinistraYInput) MyActor->TorreSinistraY = FCString::Atoi(*TorreSinistraYInput->GetText().ToString());
    if (TorreDestraXInput) MyActor->TorreDestraX = FCString::Atoi(*TorreDestraXInput->GetText().ToString());
    if (TorreDestraYInput) MyActor->TorreDestraY = FCString::Atoi(*TorreDestraYInput->GetText().ToString());

    // Rigenera la mappa
    MyActor->Griglia.Empty();
    MyActor->Griglia.SetNum(MyActor->GridSize * MyActor->GridSize);
    MyActor->GeneraGriglia();
    MyActor->PosizionaTorri();
    MyActor->VerificaRaggiungibilita();
    MyActor->PosizionaCamera();
    MyActor->MostraStatistiche();

    // Chiudi il widget
    RemoveFromParent();

    // Torna al gioco
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetShowMouseCursor(false);
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UWBP_MapConfig::OnMappaCasualeClicked()
{
    if (!GameManager || !GameManager->GrigliaAttore) return;

    AMyActor* MyActor = GameManager->GrigliaAttore;

    // Genera valori casuali
    MyActor->GridSize = FMath::RandRange(15, 35);
    MyActor->CellSize = 200.0f;
    MyActor->MaxHeight = FMath::RandRange(3, 6);
    MyActor->NumTorri = FMath::RandRange(2, 5);

    int32 Centro = MyActor->GridSize / 2;
    MyActor->TorreCentraleX = FMath::RandRange(Centro - 3, Centro + 3);
    MyActor->TorreCentraleY = FMath::RandRange(Centro - 3, Centro + 3);
    MyActor->TorreSinistraX = FMath::RandRange(2, MyActor->GridSize - 3);
    MyActor->TorreSinistraY = FMath::RandRange(2, MyActor->GridSize - 3);
    MyActor->TorreDestraX = FMath::RandRange(2, MyActor->GridSize - 3);
    MyActor->TorreDestraY = FMath::RandRange(2, MyActor->GridSize - 3);

    // Aggiorna i campi di input
    if (GridSizeInput) GridSizeInput->SetText(FText::AsNumber(MyActor->GridSize));
    if (CellSizeInput) CellSizeInput->SetText(FText::AsNumber(MyActor->CellSize));
    if (MaxHeightInput) MaxHeightInput->SetText(FText::AsNumber(MyActor->MaxHeight));
    if (NumTorriInput) NumTorriInput->SetText(FText::AsNumber(MyActor->NumTorri));

    if (TorreCentraleXInput) TorreCentraleXInput->SetText(FText::AsNumber(MyActor->TorreCentraleX));
    if (TorreCentraleYInput) TorreCentraleYInput->SetText(FText::AsNumber(MyActor->TorreCentraleY));
    if (TorreSinistraXInput) TorreSinistraXInput->SetText(FText::AsNumber(MyActor->TorreSinistraX));
    if (TorreSinistraYInput) TorreSinistraYInput->SetText(FText::AsNumber(MyActor->TorreSinistraY));
    if (TorreDestraXInput) TorreDestraXInput->SetText(FText::AsNumber(MyActor->TorreDestraX));
    if (TorreDestraYInput) TorreDestraYInput->SetText(FText::AsNumber(MyActor->TorreDestraY));
}