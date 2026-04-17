#include "WBP_HUD.h"
#include "GameManager.h"
#include "Unita.h"
#include "MyActor.h"
#include "Components/TextBlock.h"

void UWBP_HUD::AggiornaHUD()
{
    if (!GameManagerRef) return;

    // === TURNO ===
    if (Txt_NumeroTurno)
    {
        FString Testo;
        switch (GameManagerRef->FaseCorrente)
        {
        case EFaseGioco::Posizionamento:
            Testo = TEXT("POSIZIONAMENTO");
            break;
        case EFaseGioco::Gioco:
            Testo = FString::Printf(TEXT("TURNO %d"), GameManagerRef->TurnoCorrente);
            break;
        case EFaseGioco::Fine:
            Testo = TEXT("FINE PARTITA");
            break;
        default:
            Testo = TEXT("---");
            break;
        }
        Txt_NumeroTurno->SetText(FText::FromString(Testo));
    }

    if (Txt_TurnoCorrente && GameManagerRef->FaseCorrente == EFaseGioco::Gioco)
    {
        FString Testo = GameManagerRef->bTurnoGiocatore ? TEXT("TURNO GIOCATORE") : TEXT("TURNO IA");
        Txt_TurnoCorrente->SetText(FText::FromString(Testo));

        FSlateColor Colore = GameManagerRef->bTurnoGiocatore ?
            FSlateColor(FLinearColor(0.f, 1.f, 1.f)) :
            FSlateColor(FLinearColor(1.f, 0.f, 1.f));
        Txt_TurnoCorrente->SetColorAndOpacity(Colore);
    }

    // === VITA UNITÀ ===
    // Trova le unità
    AUnita* SniperGiocatore = nullptr;
    AUnita* BrawlerGiocatore = nullptr;
    AUnita* SniperIA = nullptr;
    AUnita* BrawlerIA = nullptr;

    for (AUnita* U : GameManagerRef->UnitaGiocatore)
    {
        if (!U) continue;
        if (U->TipoUnita == ETipoUnita::Sniper) SniperGiocatore = U;
        if (U->TipoUnita == ETipoUnita::Brawler) BrawlerGiocatore = U;
    }

    for (AUnita* U : GameManagerRef->UnitaIA)
    {
        if (!U) continue;
        if (U->TipoUnita == ETipoUnita::Sniper) SniperIA = U;
        if (U->TipoUnita == ETipoUnita::Brawler) BrawlerIA = U;
    }

    // Aggiorna Sniper Giocatore
    if (Txt_SniperGiocatore)
    {
        if (SniperGiocatore && SniperGiocatore->IsVivo())
        {
            FString Testo = FString::Printf(TEXT("Sniper HP %d/%d"),
                SniperGiocatore->PuntiVita, SniperGiocatore->PuntiVitaMax);
            Txt_SniperGiocatore->SetText(FText::FromString(Testo));

            float Percentuale = (float)SniperGiocatore->PuntiVita / (float)SniperGiocatore->PuntiVitaMax;
            if (Percentuale >= 0.6f)
                Txt_SniperGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.1f)));
            else if (Percentuale >= 0.3f)
                Txt_SniperGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.f)));
            else
                Txt_SniperGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.15f, 0.05f)));
        }
        else
        {
            Txt_SniperGiocatore->SetText(FText::FromString(TEXT("Sniper HP --")));
            Txt_SniperGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
        }
    }

    // Aggiorna Brawler Giocatore
    if (Txt_BrawlerGiocatore)
    {
        if (BrawlerGiocatore && BrawlerGiocatore->IsVivo())
        {
            FString Testo = FString::Printf(TEXT("Brawler HP %d/%d"),
                BrawlerGiocatore->PuntiVita, BrawlerGiocatore->PuntiVitaMax);
            Txt_BrawlerGiocatore->SetText(FText::FromString(Testo));

            float Percentuale = (float)BrawlerGiocatore->PuntiVita / (float)BrawlerGiocatore->PuntiVitaMax;
            if (Percentuale >= 0.6f)
                Txt_BrawlerGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.1f)));
            else if (Percentuale >= 0.3f)
                Txt_BrawlerGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.f)));
            else
                Txt_BrawlerGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.15f, 0.05f)));
        }
        else
        {
            Txt_BrawlerGiocatore->SetText(FText::FromString(TEXT("Brawler HP --")));
            Txt_BrawlerGiocatore->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
        }
    }

    // Aggiorna Sniper IA
    if (Txt_SniperIA)
    {
        if (SniperIA && SniperIA->IsVivo())
        {
            FString Testo = FString::Printf(TEXT("Sniper IA %d/%d"),
                SniperIA->PuntiVita, SniperIA->PuntiVitaMax);
            Txt_SniperIA->SetText(FText::FromString(Testo));

            float Percentuale = (float)SniperIA->PuntiVita / (float)SniperIA->PuntiVitaMax;
            if (Percentuale >= 0.6f)
                Txt_SniperIA->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.1f)));
            else if (Percentuale >= 0.3f)
                Txt_SniperIA->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.f)));
            else
                Txt_SniperIA->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.15f, 0.05f)));
        }
        else
        {
            Txt_SniperIA->SetText(FText::FromString(TEXT("Sniper IA --")));
            Txt_SniperIA->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
        }
    }

    // Aggiorna Brawler IA
    if (Txt_BrawlerIA)
    {
        if (BrawlerIA && BrawlerIA->IsVivo())
        {
            FString Testo = FString::Printf(TEXT("Brawler IA %d/%d"),
                BrawlerIA->PuntiVita, BrawlerIA->PuntiVitaMax);
            Txt_BrawlerIA->SetText(FText::FromString(Testo));

            float Percentuale = (float)BrawlerIA->PuntiVita / (float)BrawlerIA->PuntiVitaMax;
            if (Percentuale >= 0.6f)
                Txt_BrawlerIA->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.1f)));
            else if (Percentuale >= 0.3f)
                Txt_BrawlerIA->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.f)));
            else
                Txt_BrawlerIA->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.15f, 0.05f)));
        }
        else
        {
            Txt_BrawlerIA->SetText(FText::FromString(TEXT("Brawler IA --")));
            Txt_BrawlerIA->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
        }
    }

    // === TORRI ===
    if (GameManagerRef->GrigliaAttore)
    {
        AMyActor* Griglia = GameManagerRef->GrigliaAttore;

        // Pulisci i testi
        if (Txt_Torre1) Txt_Torre1->SetText(FText::FromString(TEXT("Torre 1: --")));
        if (Txt_Torre2) Txt_Torre2->SetText(FText::FromString(TEXT("Torre 2: --")));
        if (Txt_Torre3) Txt_Torre3->SetText(FText::FromString(TEXT("Torre 3: --")));

        for (int32 X = 0; X < Griglia->GridSize; X++)
        {
            for (int32 Y = 0; Y < Griglia->GridSize; Y++)
            {
                int32 Idx = X * Griglia->GridSize + Y;
                if (Griglia->Griglia.IsValidIndex(Idx) && Griglia->Griglia[Idx].bIsTower)
                {
                    int32 ID = Griglia->Griglia[Idx].TowerID;
                    FString Stato;
                    FSlateColor Colore;

                    switch (Griglia->Griglia[Idx].StatoTorre)
                    {
                    case EStatoTorre::Neutrale:
                        Stato = TEXT("NEUTRALE");
                        Colore = FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));
                        break;
                    case EStatoTorre::ControlloP1:
                        Stato = TEXT("GIOCATORE");
                        Colore = FSlateColor(FLinearColor(0.f, 1.f, 1.f));
                        break;
                    case EStatoTorre::ControlloIA:
                        Stato = TEXT("IA");
                        Colore = FSlateColor(FLinearColor(1.f, 0.f, 1.f));
                        break;
                    case EStatoTorre::Contesa:
                        Stato = TEXT("CONTESA");
                        Colore = FSlateColor(FLinearColor(1.f, 0.9f, 0.f));
                        break;
                    default:
                        Stato = TEXT("?");
                        Colore = FSlateColor(FLinearColor(1.f, 1.f, 1.f));
                        break;
                    }

                    FString Testo = FString::Printf(TEXT("Torre %d: %s"), ID, *Stato);

                    if (ID == 1 && Txt_Torre1)
                    {
                        Txt_Torre1->SetText(FText::FromString(Testo));
                        Txt_Torre1->SetColorAndOpacity(Colore);
                    }
                    else if (ID == 2 && Txt_Torre2)
                    {
                        Txt_Torre2->SetText(FText::FromString(Testo));
                        Txt_Torre2->SetColorAndOpacity(Colore);
                    }
                    else if (ID == 3 && Txt_Torre3)
                    {
                        Txt_Torre3->SetText(FText::FromString(Testo));
                        Txt_Torre3->SetColorAndOpacity(Colore);
                    }
                }
            }
        }
    }
}