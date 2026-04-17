#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "WBP_MapConfig.generated.h"

/**
 * Widget di configurazione per la mappa
 */
UCLASS()
class GAME_API UWBP_MapConfig : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Campi di input - questi si collegheranno automaticamente con i widget nell'editor
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* GridSizeInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* CellSizeInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* MaxHeightInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* NumTorriInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreCentraleXInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreCentraleYInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreSinistraXInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreSinistraYInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreDestraXInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* TorreDestraYInput;

    // Bottoni
    UPROPERTY(meta = (BindWidget))
    class UButton* GeneraMappaButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* MappaCasualeButton;

    // Funzioni per i click
    UFUNCTION()
    void OnGeneraMappaClicked();

    UFUNCTION()
    void OnMappaCasualeClicked();

private:
    class AGameManager* GameManager;
};