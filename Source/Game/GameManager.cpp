#include "GameManager.h"
#include "MyActor.h"
#include "Unita.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AGameManager::AGameManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGameManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("=== GAME MANAGER INIZIALIZZATO ==="));
    GrigliaAttore = Cast<AMyActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyActor::StaticClass()));
    if (!GrigliaAttore)
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: Nessun MyActor trovato nel livello!"));
        return;
    }
    InizializzaGioco();
}

void AGameManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FaseCorrente == EFaseGioco::Gioco && !bTurnoGiocatore && !bFineTurnoInCorso)
    {
        bool bTuttiFermi = true;
        bool bAlmenoUnaViva = false;

        for (AUnita* Unita : UnitaIA)
        {
            if (Unita && Unita->IsVivo())
            {
                bAlmenoUnaViva = true;
                if (Unita->bSiStaMuovendo)
                {
                    bTuttiFermi = false;
                    break;
                }
            }
        }

        if (bTuttiFermi && bAlmenoUnaViva)
        {
            bool bTutteAgito = true;
            for (AUnita* Unita : UnitaIA)
            {
                if (Unita && Unita->IsVivo() && !Unita->bHaAgitoNelTurno)
                {
                    bTutteAgito = false;
                    break;
                }
            }
            if (bTutteAgito)
            {
                UE_LOG(LogTemp, Warning, TEXT("Tutte le unita IA hanno agito. Fine turno automatico."));
                bFineTurnoInCorso = true;
                FineTurno();
            }
        }
    }
}

void AGameManager::InizializzaGioco()
{
    UE_LOG(LogTemp, Warning, TEXT("Inizializzazione gioco..."));

    TArray<AActor*> UnitaEsistenti;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnita::StaticClass(), UnitaEsistenti);
    if (UnitaEsistenti.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trovate %d unita esistenti. Eliminazione..."), UnitaEsistenti.Num());
        for (AActor* Unita : UnitaEsistenti)
            if (Unita && !Unita->IsPendingKillPending())
                Unita->Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna unita esistente trovata nel livello."));
    }

    UnitaDaPosizionareGiocatore.Empty();
    UnitaDaPosizionareIA.Empty();
    UnitaGiocatore.Empty();
    UnitaIA.Empty();
    CelleEvidenziate.Empty();
    StoricoMosse.Empty();
    PosizioniInizialiGiocatore.Empty();
    PosizioniInizialiIA.Empty();

    PuntiGiocatore = 0;
    PuntiIA = 0;
    PuntiGiocatoreTurnoPrecedente = 0;
    PuntiIATurnoPrecedente = 0;
    TurniControlloGiocatore = 0;
    TurniControlloIA = 0;
    bFineTurnoInCorso = false;

    CreaUnitaIniziali();
    FaseCorrente = EFaseGioco::LancioMoneta;
    LancioMoneta();
}

void AGameManager::CreaUnitaIniziali()
{
    UnitaDaPosizionareGiocatore.Add(ETipoUnita::Sniper);
    UnitaDaPosizionareGiocatore.Add(ETipoUnita::Brawler);
    UnitaDaPosizionareIA.Add(ETipoUnita::Sniper);
    UnitaDaPosizionareIA.Add(ETipoUnita::Brawler);
    UE_LOG(LogTemp, Warning, TEXT("Unita create: 2 per giocatore, 2 per IA"));
}

void AGameManager::LancioMoneta()
{
    UE_LOG(LogTemp, Warning, TEXT("=== LANCIO DELLA MONETA ==="));
    bVincitoreLancio = FMath::RandBool();
    if (bVincitoreLancio)
    {
        UE_LOG(LogTemp, Warning, TEXT("Il GIOCATORE vince il lancio!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("L'IA vince il lancio!"));
    }
    IniziaPosizionamento();
}

void AGameManager::IniziaPosizionamento()
{
    FaseCorrente = EFaseGioco::Posizionamento;
    UE_LOG(LogTemp, Warning, TEXT("=== FASE DI POSIZIONAMENTO ==="));

    if (bVincitoreLancio)
    {
        bTurnoGiocatore = true;
        UE_LOG(LogTemp, Warning, TEXT("Turno del GIOCATORE - Posiziona una unita (righe %d-%d)"),
            ZonaSchieramentoGiocatoreMin, ZonaSchieramentoGiocatoreMax);
    }
    else
    {
        bTurnoGiocatore = false;
        UE_LOG(LogTemp, Warning, TEXT("Turno dell'IA - Posiziona una unita (righe %d-%d)"),
            ZonaSchieramentoIAMin, ZonaSchieramentoIAMax);
        GetWorld()->GetTimerManager().SetTimer(TimerHandleIA, this, &AGameManager::PosizionamentoIA, 1.0f, false);
    }

    MostraFaseCorrente();
    if (WidgetHUD)
    {
        WidgetHUD->AggiornaHUD();
    }
}

void AGameManager::PosizionamentoIA()
{
    if (FaseCorrente != EFaseGioco::Posizionamento || bTurnoGiocatore) return;
    if (UnitaDaPosizionareIA.Num() == 0) { PassaProssimoTurnoPosizionamento(); return; }

    TArray<FIntPoint> CelleLibere = GetCelleLiberePerPosizionamento();
    if (CelleLibere.Num() == 0) { UE_LOG(LogTemp, Error, TEXT("Nessuna cella libera per IA!")); return; }

    int32 RandomIndex = FMath::RandRange(0, CelleLibere.Num() - 1);
    FIntPoint Pos = CelleLibere[RandomIndex];
    ETipoUnita TipoDaPosizionare = UnitaDaPosizionareIA[0];
    UnitaDaPosizionareIA.RemoveAt(0);

    PosizionaUnita(false, TipoDaPosizionare, Pos.X, Pos.Y);
    PassaProssimoTurnoPosizionamento();
}

TArray<FIntPoint> AGameManager::GetCelleLiberePerPosizionamento() const
{
    TArray<FIntPoint> CelleLibere;
    if (!GrigliaAttore) return CelleLibere;

    for (const FIntPoint& Cella : GrigliaAttore->GetCelleCalpestabili())
    {
        if (bTurnoGiocatore)
        {
            if (Cella.Y >= ZonaSchieramentoGiocatoreMin && Cella.Y <= ZonaSchieramentoGiocatoreMax)
                if (!IsCellaOccupata(Cella.X, Cella.Y))
                    CelleLibere.Add(Cella);
        }
        else
        {
            if (Cella.Y >= ZonaSchieramentoIAMin && Cella.Y <= ZonaSchieramentoIAMax)
                if (!IsCellaOccupata(Cella.X, Cella.Y))
                    CelleLibere.Add(Cella);
        }
    }
    return CelleLibere;
}

void AGameManager::PosizionaUnita(bool bGiocatore, ETipoUnita Tipo, int32 X, int32 Y)
{
    if (!GrigliaAttore) return;
    if (!GrigliaAttore->IsCellaCalpestabile(X, Y) || IsCellaOccupata(X, Y))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella (%d,%d) non valida!"), X, Y);
        return;
    }
    if (bGiocatore && (Y < ZonaSchieramentoGiocatoreMin || Y > ZonaSchieramentoGiocatoreMax))
    {
        UE_LOG(LogTemp, Warning, TEXT("Fuori zona schieramento giocatore!"));
        return;
    }
    if (!bGiocatore && (Y < ZonaSchieramentoIAMin || Y > ZonaSchieramentoIAMax))
    {
        UE_LOG(LogTemp, Warning, TEXT("Fuori zona schieramento IA!"));
        return;
    }

    CreaUnitaEffettiva(bGiocatore, Tipo, X, Y);
    if (WidgetHUD)
    {
        WidgetHUD->AggiornaHUD();
    }
}

void AGameManager::CreaUnitaEffettiva(bool bGiocatore, ETipoUnita Tipo, int32 X, int32 Y)
{
    if (!GrigliaAttore) { UE_LOG(LogTemp, Error, TEXT("GrigliaAttore NULL!")); return; }

    float CellSize = GrigliaAttore->CellSize;
    float GridSize = GrigliaAttore->GridSize;
    float MetaGrid = GridSize / 2.0f;
    float XMondo = (X - MetaGrid) * CellSize + CellSize / 2;
    float YMondo = (Y - MetaGrid) * CellSize + CellSize / 2;
    int32 Index = X * (int32)GridSize + Y;
    float Z = 300.0f;
    if (GrigliaAttore->Griglia.IsValidIndex(Index))
        Z = GrigliaAttore->Griglia[Index].Z + 200.0f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AUnita* NuovaUnita = GetWorld()->SpawnActor<AUnita>(
        AUnita::StaticClass(), FVector(XMondo, YMondo, Z), FRotator::ZeroRotator, SpawnParams);

    if (NuovaUnita)
    {
        NuovaUnita->bIsGiocatore = bGiocatore;
        NuovaUnita->TipoUnita = Tipo;
        NuovaUnita->ColoreUnita = GetColoreUnita(bGiocatore);
        NuovaUnita->ImpostaDannoPerTipo();
        NuovaUnita->SetPosizione(X, Y);
        NuovaUnita->GameManager = this;

        if (bGiocatore)
        {
            UnitaGiocatore.Add(NuovaUnita);
            PosizioniInizialiGiocatore.Add(FIntPoint(X, Y));
        }
        else
        {
            UnitaIA.Add(NuovaUnita);
            PosizioniInizialiIA.Add(FIntPoint(X, Y));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: Impossibile spawnare unita!"));
    }
}

void AGameManager::PassaProssimoTurnoPosizionamento()
{
    if (UnitaDaPosizionareGiocatore.Num() == 0 && UnitaDaPosizionareIA.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutte le unita posizionate! Inizio gioco..."));
        IniziaGioco();
        return;
    }

    bTurnoGiocatore = !bTurnoGiocatore;

    if (!bTurnoGiocatore && UnitaDaPosizionareIA.Num() > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandleIA, this, &AGameManager::PosizionamentoIA, 1.0f, false);
    }
    else if (bTurnoGiocatore && UnitaDaPosizionareGiocatore.Num() > 0)
    {
    }
    else
    {
        PassaProssimoTurnoPosizionamento();
    }

    MostraFaseCorrente();
    if (WidgetHUD) WidgetHUD->AggiornaHUD();
}

void AGameManager::IniziaGioco()
{
    FaseCorrente = EFaseGioco::Gioco;
    bTurnoGiocatore = bVincitoreLancio;
    bFineTurnoInCorso = false;

    UE_LOG(LogTemp, Warning, TEXT("=== INIZIO GIOCO ==="));
    UE_LOG(LogTemp, Warning, TEXT("Inizia: %s"), bTurnoGiocatore ? TEXT("GIOCATORE") : TEXT("IA"));

    MostraFaseCorrente();
    if (WidgetHUD) WidgetHUD->AggiornaHUD();
    IniziaTurno();
}

void AGameManager::IniziaTurno()
{
    if (FaseCorrente != EFaseGioco::Gioco) return;

    UE_LOG(LogTemp, Warning, TEXT("=== TURNO %d - %s ==="),
        TurnoCorrente, bTurnoGiocatore ? TEXT("GIOCATORE") : TEXT("IA"));

    if (WidgetHUD) WidgetHUD->AggiornaHUD();

    TArray<AUnita*>& UnitaCorrenti = bTurnoGiocatore ? UnitaGiocatore : UnitaIA;
    for (AUnita* Unita : UnitaCorrenti)
    {
        if (Unita && Unita->IsVivo() && !Unita->bSiStaMuovendo)
        {
            Unita->bHaMosso = false;
            Unita->bHaAttaccato = false;
            Unita->bHaAgitoNelTurno = false;
        }
    }

    if (bTurnoGiocatore)
    {
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandleIA, this, &AGameManager::TurnoIA, 1.0f, false);
    }
}

void AGameManager::FineTurno()
{
    if (FaseCorrente != EFaseGioco::Gioco) return;

    bFineTurnoInCorso = false;
    UE_LOG(LogTemp, Warning, TEXT("=== FINE TURNO %d ==="), TurnoCorrente);

    AggiornaStatoTorri();
    RespawnUnitaMorte();

    TArray<AUnita*>& UnitaCorrenti = bTurnoGiocatore ? UnitaGiocatore : UnitaIA;
    for (AUnita* Unita : UnitaCorrenti)
    {
        if (Unita)
        {
            Unita->bHaMosso = false;
            Unita->bHaAttaccato = false;
            Unita->bHaAgitoNelTurno = false;
        }
    }

    bTurnoGiocatore = !bTurnoGiocatore;
    if (bTurnoGiocatore) TurnoCorrente++;

    UnitaSelezionata = nullptr;
    StatoInputGiocatore = 0;
    CelleRangeMovimento.Empty();
    CelleRangeAttacco.Empty();
    PulisciEvidenziazioni();

    ControllaVittoria();
    if (FaseCorrente == EFaseGioco::Gioco) IniziaTurno();
    if (WidgetHUD) WidgetHUD->AggiornaHUD();
}

TArray<FIntPoint> AGameManager::GetCelleZonaCattura(int32 TorreX, int32 TorreY) const
{
    TArray<FIntPoint> Zona;
    if (!GrigliaAttore) return Zona;
    for (int32 DX = -2; DX <= 2; DX++)
        for (int32 DY = -2; DY <= 2; DY++)
            if (abs(DX) + abs(DY) <= 2)
            {
                int32 X = TorreX + DX;
                int32 Y = TorreY + DY;
                if (X >= 0 && X < GrigliaAttore->GridSize && Y >= 0 && Y < GrigliaAttore->GridSize)
                    Zona.Add(FIntPoint(X, Y));
            }
    return Zona;
}

void AGameManager::ControllaZonaTorre(int32 TorreX, int32 TorreY)
{
    if (!GrigliaAttore) return;
    int32 Index = TorreX * GrigliaAttore->GridSize + TorreY;
    if (!GrigliaAttore->Griglia.IsValidIndex(Index)) return;

    TArray<FIntPoint> Zona = GetCelleZonaCattura(TorreX, TorreY);
    bool bGiocatoreInZona = false;
    bool bIAInZona = false;

    for (AUnita* Unita : UnitaGiocatore)
    {
        if (!Unita || !Unita->IsVivo() || Unita->bSiStaMuovendo) continue;
        if (Zona.Contains(FIntPoint(Unita->PosX, Unita->PosY))) { bGiocatoreInZona = true; break; }
    }
    for (AUnita* Unita : UnitaIA)
    {
        if (!Unita || !Unita->IsVivo() || Unita->bSiStaMuovendo) continue;
        if (Zona.Contains(FIntPoint(Unita->PosX, Unita->PosY))) { bIAInZona = true; break; }
    }

    FCella& CellaTorre = GrigliaAttore->Griglia[Index];
    EStatoTorre StatoPrecedente = CellaTorre.StatoTorre;

    if (!bGiocatoreInZona && !bIAInZona)       CellaTorre.StatoTorre = EStatoTorre::Neutrale;
    else if (bGiocatoreInZona && !bIAInZona)   CellaTorre.StatoTorre = EStatoTorre::ControlloP1;
    else if (!bGiocatoreInZona && bIAInZona)   CellaTorre.StatoTorre = EStatoTorre::ControlloIA;
    else                                        CellaTorre.StatoTorre = EStatoTorre::Contesa;

    CellaTorre.PuntiTorre = (CellaTorre.StatoTorre == EStatoTorre::ControlloP1) ? 1 :
        (CellaTorre.StatoTorre == EStatoTorre::ControlloIA) ? 2 : 0;

    if (StatoPrecedente != CellaTorre.StatoTorre)
    {
        FString NomeStato;
        switch (CellaTorre.StatoTorre)
        {
        case EStatoTorre::Neutrale:    NomeStato = TEXT("NEUTRALE"); break;
        case EStatoTorre::ControlloP1: NomeStato = TEXT("GIOCATORE"); break;
        case EStatoTorre::ControlloIA: NomeStato = TEXT("IA"); break;
        case EStatoTorre::Contesa:     NomeStato = TEXT("CONTESA"); break;
        default:                       NomeStato = TEXT("?"); break;
        }
        UE_LOG(LogTemp, Warning, TEXT("Torre (%d,%d) ora in stato: %s"), TorreX, TorreY, *NomeStato);
    }
}

void AGameManager::AggiornaStatoTorri()
{
    if (!GrigliaAttore) return;

    PuntiGiocatoreTurnoPrecedente = PuntiGiocatore;
    PuntiIATurnoPrecedente = PuntiIA;
    PuntiGiocatore = 0;
    PuntiIA = 0;

    for (int32 X = 0; X < GrigliaAttore->GridSize; X++)
        for (int32 Y = 0; Y < GrigliaAttore->GridSize; Y++)
        {
            int32 Index = X * GrigliaAttore->GridSize + Y;
            if (GrigliaAttore->Griglia[Index].bIsTower)
            {
                ControllaZonaTorre(X, Y);
                if (GrigliaAttore->Griglia[Index].StatoTorre == EStatoTorre::ControlloP1) PuntiGiocatore++;
                else if (GrigliaAttore->Griglia[Index].StatoTorre == EStatoTorre::ControlloIA) PuntiIA++;
            }
        }

    TurniControlloGiocatore = (PuntiGiocatore >= 2) ?
        ((PuntiGiocatoreTurnoPrecedente >= 2) ? TurniControlloGiocatore + 1 : 1) : 0;
    TurniControlloIA = (PuntiIA >= 2) ?
        ((PuntiIATurnoPrecedente >= 2) ? TurniControlloIA + 1 : 1) : 0;
}

FColor AGameManager::GetColoreTorre(EStatoTorre Stato) const
{
    switch (Stato)
    {
    case EStatoTorre::Neutrale:    return FColor(200, 200, 200);
    case EStatoTorre::ControlloP1: return ColoreGiocatore;
    case EStatoTorre::ControlloIA: return ColoreIA;
    case EStatoTorre::Contesa:
    {
        float Time = GetWorld()->GetTimeSeconds();
        return (FMath::FloorToInt(Time * 2) % 2 == 0) ? ColoreGiocatore : ColoreIA;
    }
    default: return FColor::White;
    }
}

void AGameManager::NascondiRangeMovimento()
{
    bMostraRangeMovimento = false;
    PulisciEvidenziazioni();
}

void AGameManager::AggiungiMossaAStorico(FString Mossa)
{
    StoricoMosse.Add(Mossa);
    if (StoricoMosse.Num() > 20) StoricoMosse.RemoveAt(0);
    UE_LOG(LogTemp, Warning, TEXT("STORICO: %s"), *Mossa);
    if (WidgetHUD) WidgetHUD->AggiornaHUD();
}

FString AGameManager::GetStoricoMosse() const
{
    FString Risultato;
    for (const FString& Mossa : StoricoMosse)
        Risultato += Mossa + TEXT("\n");
    return Risultato;
}

int32 AGameManager::CalcolaDannoContrattacco(AUnita* Attaccante, AUnita* Difensore, int32 Distanza)
{
    if (!Attaccante || !Difensore || Attaccante->TipoUnita != ETipoUnita::Sniper) return 0;

    bool bContrattacco = (Difensore->TipoUnita == ETipoUnita::Sniper) ||
        (Difensore->TipoUnita == ETipoUnita::Brawler && Distanza == 1);
    if (!bContrattacco) return 0;

    int32 Danno = FMath::RandRange(1, 3);
    UE_LOG(LogTemp, Warning, TEXT("CONTRATTACCO! Sniper %s subisce %d danni (dist=%d)"),
        Attaccante->bIsGiocatore ? TEXT("Giocatore") : TEXT("IA"), Danno, Distanza);
    return Danno;
}

bool AGameManager::ValutaPrioritaTorri(AUnita* Unita, FIntPoint& MiglioreTorre)
{
    if (!Unita || !GrigliaAttore) return false;
    int32 MiglioreDistanza = 9999;
    bool bTrovato = false;

    for (int32 X = 0; X < GrigliaAttore->GridSize; X++)
        for (int32 Y = 0; Y < GrigliaAttore->GridSize; Y++)
        {
            int32 Index = X * GrigliaAttore->GridSize + Y;
            if (GrigliaAttore->Griglia[Index].bIsTower &&
                GrigliaAttore->Griglia[Index].StatoTorre != EStatoTorre::ControlloP1 &&
                GrigliaAttore->Griglia[Index].StatoTorre != EStatoTorre::ControlloIA)
            {
                int32 Distanza = GrigliaAttore->DistanzaManhattan(Unita->PosX, Unita->PosY, X, Y);
                if (Distanza < MiglioreDistanza)
                {
                    MiglioreDistanza = Distanza;
                    MiglioreTorre = FIntPoint(X, Y);
                    bTrovato = true;
                }
            }
        }
    return bTrovato;
}

void AGameManager::RespawnUnitaMorte()
{
    if (!GrigliaAttore) return;
    TArray<FUnitaRespawnInfo> UniteDaRespawnare;

    for (int32 i = UnitaGiocatore.Num() - 1; i >= 0; i--)
    {
        if (!UnitaGiocatore[i] || !UnitaGiocatore[i]->IsVivo())
        {
            if (UnitaGiocatore[i])
            {
                ETipoUnita Tipo = UnitaGiocatore[i]->TipoUnita;
                FIntPoint PosIniziale = PosizioniInizialiGiocatore.IsValidIndex(i) ?
                    PosizioniInizialiGiocatore[i] : FIntPoint(-1, -1);
                if (PosIniziale.X != -1)
                    UniteDaRespawnare.Add(FUnitaRespawnInfo(true, Tipo, PosIniziale.X, PosIniziale.Y));
                AUnita* UnitaMorta = UnitaGiocatore[i];
                UnitaGiocatore.RemoveAt(i);
                PosizioniInizialiGiocatore.RemoveAt(i);
                if (UnitaMorta && !UnitaMorta->IsPendingKillPending()) UnitaMorta->Destroy();
            }
        }
    }

    for (int32 i = UnitaIA.Num() - 1; i >= 0; i--)
    {
        if (!UnitaIA[i] || !UnitaIA[i]->IsVivo())
        {
            if (UnitaIA[i])
            {
                ETipoUnita Tipo = UnitaIA[i]->TipoUnita;
                FIntPoint PosIniziale = PosizioniInizialiIA.IsValidIndex(i) ?
                    PosizioniInizialiIA[i] : FIntPoint(-1, -1);
                if (PosIniziale.X != -1)
                    UniteDaRespawnare.Add(FUnitaRespawnInfo(false, Tipo, PosIniziale.X, PosIniziale.Y));
                AUnita* UnitaMorta = UnitaIA[i];
                UnitaIA.RemoveAt(i);
                PosizioniInizialiIA.RemoveAt(i);
                if (UnitaMorta && !UnitaMorta->IsPendingKillPending()) UnitaMorta->Destroy();
            }
        }
    }

    for (const FUnitaRespawnInfo& Info : UniteDaRespawnare)
        CreaUnitaEffettiva(Info.bGiocatore, Info.Tipo, Info.X, Info.Y);

    if (UniteDaRespawnare.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Respawn: create %d nuove unita (Giocatore: %d, IA: %d)"),
            UniteDaRespawnare.Num(), UnitaGiocatore.Num(), UnitaIA.Num());
    }
}

void AGameManager::SelezionaUnita(AUnita* Unita)
{
    if (FaseCorrente != EFaseGioco::Gioco || !Unita || !Unita->IsVivo() || Unita->bSiStaMuovendo) return;

    if (bTurnoGiocatore && Unita->bIsGiocatore)
    {
        if (Unita->bHaAgitoNelTurno)
        {
            UE_LOG(LogTemp, Warning, TEXT("Questa unita ha gia agito questo turno!"));
            return;
        }
        UnitaSelezionata = Unita;
        EvidenziaCella(Unita->PosX, Unita->PosY, FColor::White);
        bMostraRangeMovimento = true;

        bool bPuoMuovere = PuoMuoversiUnita(Unita);
        bool bPuoAttaccare = PuoAttaccareUnita(Unita);

        if (bPuoMuovere && GrigliaAttore)
        {
            TArray<FIntPoint> CelleRaggiungibili = GetCelleRaggiungibiliPerUnitaSelezionata();
            GrigliaAttore->CelleMovimentoEvidenziate = CelleRaggiungibili;
            GrigliaAttore->bMostraHighlight = true;
        }

        if (bPuoAttaccare && GrigliaAttore)
        {
            TArray<AUnita*> Nemici = GetUnitaNemicheNelRaggio(Unita);
            GrigliaAttore->CelleAttaccoEvidenziate.Empty();
            for (AUnita* N : Nemici)
                if (N && N->IsVivo())
                    GrigliaAttore->CelleAttaccoEvidenziate.Add(FIntPoint(N->PosX, N->PosY));
            GrigliaAttore->bMostraHighlight = true;
        }
    }
}

void AGameManager::EvidenziaCella(int32 X, int32 Y, FColor Colore)
{
    if (!GrigliaAttore) return;
    CelleEvidenziate.Add(FIntPoint(X, Y));
    if (Colore == FColor::White)
        GrigliaAttore->CellaUnitaSelezionata = FIntPoint(X, Y);
    else if (Colore == FColor::Red)
        GrigliaAttore->CelleAttaccoEvidenziate.AddUnique(FIntPoint(X, Y));
    GrigliaAttore->bMostraHighlight = true;
}

void AGameManager::PulisciEvidenziazioni()
{
    CelleEvidenziate.Empty();
    if (GrigliaAttore) GrigliaAttore->PulisciHighlight();
}

bool AGameManager::PuoMuoversiUnita(AUnita* Unita)
{
    if (!Unita || !Unita->IsVivo() || Unita->bSiStaMuovendo) return false;
    TArray<FIntPoint> CelleRaggiungibili = GrigliaAttore->GetCelleRaggiungibili(
        Unita->PosX, Unita->PosY, Unita->Movimento);
    for (const FIntPoint& Cella : CelleRaggiungibili)
        if (!IsCellaOccupata(Cella.X, Cella.Y)) return true;
    return false;
}

bool AGameManager::PuoAttaccareUnita(AUnita* Unita)
{
    if (!Unita || !Unita->IsVivo() || Unita->bSiStaMuovendo) return false;
    return GetUnitaNemicheNelRaggio(Unita).Num() > 0;
}

TArray<FIntPoint> AGameManager::GetCelleRaggiungibiliPerUnitaSelezionata()
{
    TArray<FIntPoint> CelleLibere;
    if (!UnitaSelezionata || !GrigliaAttore || FaseCorrente != EFaseGioco::Gioco) return CelleLibere;

    TArray<FIntPoint> CelleRaggiungibili = GrigliaAttore->GetCelleRaggiungibili(
        UnitaSelezionata->PosX, UnitaSelezionata->PosY, UnitaSelezionata->Movimento);
    for (const FIntPoint& Cella : CelleRaggiungibili)
        if (!IsCellaOccupata(Cella.X, Cella.Y))
            CelleLibere.Add(Cella);
    return CelleLibere;
}

void AGameManager::MuoviUnitaSelezionata(int32 DestX, int32 DestY)
{
    if (!UnitaSelezionata || !GrigliaAttore || FaseCorrente != EFaseGioco::Gioco) return;

    TArray<FIntPoint> CelleRaggiungibiliCalc = GrigliaAttore->GetCelleRaggiungibili(
        UnitaSelezionata->PosX, UnitaSelezionata->PosY, UnitaSelezionata->Movimento);
    TArray<FIntPoint> CelleLibere;
    for (const FIntPoint& Cella : CelleRaggiungibiliCalc)
        if (!IsCellaOccupata(Cella.X, Cella.Y))
            CelleLibere.Add(Cella);

    if (!CelleLibere.Contains(FIntPoint(DestX, DestY)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella (%d,%d) non raggiungibile!"), DestX, DestY);
        return;
    }

    TArray<FIntPoint> Percorso = GrigliaAttore->GetPercorso(
        UnitaSelezionata->PosX, UnitaSelezionata->PosY, DestX, DestY, UnitaSelezionata->Movimento);

    FString CoordPartenza = GrigliaAttore->GetCoordinateString(UnitaSelezionata->PosX, UnitaSelezionata->PosY);
    FString CoordDestinazione = GrigliaAttore->GetCoordinateString(DestX, DestY);
    FString Mossa = UnitaSelezionata->bIsGiocatore ? TEXT("HP: ") : TEXT("AI: ");
    Mossa += (UnitaSelezionata->TipoUnita == ETipoUnita::Sniper) ? TEXT("S ") : TEXT("B ");
    Mossa += FString::Printf(TEXT("%s -> %s"), *CoordPartenza, *CoordDestinazione);
    AggiungiMossaAStorico(Mossa);

    if (Percorso.Num() > 0)
        UnitaSelezionata->IniziaMovimentoAnimato(Percorso);
    else
    {
        UnitaSelezionata->SetPosizione(DestX, DestY);
        UnitaSelezionata->bHaAgitoNelTurno = true;
    }

    UnitaSelezionata->bHaMosso = true;

    PulisciEvidenziazioni();
    CelleRangeMovimento.Empty();
    StatoInputGiocatore = 2;

    AUnita* UnitaDaMostrare = UnitaSelezionata;
    FTimerHandle TimerRangeAttacco;
    GetWorld()->GetTimerManager().SetTimer(TimerRangeAttacco, [this, UnitaDaMostrare]()
        {
            if (!UnitaDaMostrare || !UnitaDaMostrare->IsVivo() || UnitaDaMostrare->bSiStaMuovendo) return;
            if (!GrigliaAttore || StatoInputGiocatore != 2) return;
            TArray<AUnita*> Nemici = GetUnitaNemicheNelRaggio(UnitaDaMostrare);
            if (Nemici.Num() > 0)
            {
                GrigliaAttore->CelleAttaccoEvidenziate.Empty();
                for (AUnita* N : Nemici)
                    if (N && N->IsVivo())
                        GrigliaAttore->CelleAttaccoEvidenziate.Add(FIntPoint(N->PosX, N->PosY));
                GrigliaAttore->CellaUnitaSelezionata = FIntPoint(UnitaDaMostrare->PosX, UnitaDaMostrare->PosY);
                GrigliaAttore->bMostraHighlight = true;
                UE_LOG(LogTemp, Warning, TEXT("Range attacco disponibile. Clicca su un nemico o cella vuota per terminare."));
            }
        }, 0.5f, false);

    if (WidgetHUD) WidgetHUD->AggiornaHUD();
}

void AGameManager::AttaccaConUnitaSelezionata(AUnita* Bersaglio)
{
    if (!UnitaSelezionata || !Bersaglio || FaseCorrente != EFaseGioco::Gioco) return;

    int32 BersaglioX = Bersaglio->PosX;
    int32 BersaglioY = Bersaglio->PosY;
    int32 AttaccanteX = UnitaSelezionata->PosX;
    int32 AttaccanteY = UnitaSelezionata->PosY;

    if (GrigliaAttore)
    {
        GrigliaAttore->CelleAttaccoEvidenziate.Empty();
        GrigliaAttore->CelleAttaccoEvidenziate.Add(FIntPoint(BersaglioX, BersaglioY));
        GrigliaAttore->CellaUnitaSelezionata = FIntPoint(AttaccanteX, AttaccanteY);
        GrigliaAttore->bMostraHighlight = true;

        FTimerHandle TimerHL;
        GetWorld()->GetTimerManager().SetTimer(TimerHL, [this]()
            {
                if (GrigliaAttore) GrigliaAttore->PulisciHighlight();
            }, 1.5f, false);
    }

    if (GrigliaAttore->EseguiAttacco(UnitaSelezionata, Bersaglio))
    {
        FString CoordDestinazione = GrigliaAttore->GetCoordinateString(BersaglioX, BersaglioY);
        int32 DannoInflitto = UnitaSelezionata->CalcolaDanno();
        FString Mossa = UnitaSelezionata->bIsGiocatore ? TEXT("HP: ") : TEXT("AI: ");
        Mossa += (UnitaSelezionata->TipoUnita == ETipoUnita::Sniper) ? TEXT("S ") : TEXT("B ");
        Mossa += FString::Printf(TEXT("%s %d"), *CoordDestinazione, DannoInflitto);
        AggiungiMossaAStorico(Mossa);

        UnitaSelezionata->bHaAttaccato = true;
        UnitaSelezionata->bHaAgitoNelTurno = true;

        PulisciEvidenziazioni();
        UnitaSelezionata = nullptr;
        StatoInputGiocatore = 0;
        ControllaFineTurno();
        if (WidgetHUD) WidgetHUD->AggiornaHUD();
    }
    else
    {
        if (GrigliaAttore) GrigliaAttore->PulisciHighlight();
    }
}

void AGameManager::ControllaFineTurno()
{
    if (FaseCorrente != EFaseGioco::Gioco) return;

    TArray<AUnita*>& UnitaCorrenti = bTurnoGiocatore ? UnitaGiocatore : UnitaIA;
    bool bAlmenoUnaViva = false;

    for (AUnita* Unita : UnitaCorrenti)
    {
        if (Unita && Unita->IsVivo())
        {
            bAlmenoUnaViva = true;
            if (Unita->bSiStaMuovendo) return;
        }
    }

    if (!bAlmenoUnaViva) { FineTurno(); return; }

    for (AUnita* Unita : UnitaCorrenti)
        if (Unita && Unita->IsVivo() && !Unita->bHaAgitoNelTurno) return;

    UE_LOG(LogTemp, Warning, TEXT("Tutte le unita hanno agito. Fine turno."));

    if (bTurnoGiocatore)
    {
        UnitaSelezionata = nullptr;
        StatoInputGiocatore = 0;
        CelleRangeMovimento.Empty();
        CelleRangeAttacco.Empty();
        PulisciEvidenziazioni();
        UE_LOG(LogTemp, Warning, TEXT("Turno giocatore completato. Passaggio automatico all'IA..."));
    }

    FineTurno();
}

TArray<AUnita*> AGameManager::GetUnitaNemicheNelRaggio(AUnita* Attaccante)
{
    TArray<AUnita*> NemiciNelRaggio;
    if (!Attaccante || !GrigliaAttore || Attaccante->bSiStaMuovendo) return NemiciNelRaggio;

    TArray<FIntPoint> PosizioniNelRaggio = GrigliaAttore->GetUnitaNelRaggio(
        Attaccante->PosX, Attaccante->PosY, Attaccante->TipoAttacco, Attaccante->RangeAttacco);
    TArray<AUnita*>& UnitaNemiche = Attaccante->bIsGiocatore ? UnitaIA : UnitaGiocatore;

    for (AUnita* Nemico : UnitaNemiche)
    {
        if (!Nemico || !Nemico->IsVivo() || Nemico->bSiStaMuovendo) continue;
        if (PosizioniNelRaggio.Contains(FIntPoint(Nemico->PosX, Nemico->PosY)))
        {
            int32 LivelloA = GrigliaAttore->Griglia[Attaccante->PosX * GrigliaAttore->GridSize + Attaccante->PosY].Altezza;
            int32 LivelloD = GrigliaAttore->Griglia[Nemico->PosX * GrigliaAttore->GridSize + Nemico->PosY].Altezza;
            if (LivelloD <= LivelloA) NemiciNelRaggio.Add(Nemico);
        }
    }

    NemiciNelRaggio.Sort([&](const AUnita& A, const AUnita& B)
        {
            int32 DistA = GrigliaAttore->DistanzaManhattan(Attaccante->PosX, Attaccante->PosY, A.PosX, A.PosY);
            int32 DistB = GrigliaAttore->DistanzaManhattan(Attaccante->PosX, Attaccante->PosY, B.PosX, B.PosY);
            return DistA < DistB;
        });

    return NemiciNelRaggio;
}

float AGameManager::ValutaCellaIA(AUnita* Unita, int32 CX, int32 CY) const
{
    float Score = 0.f;
    const float PESO_TORRE_NEUTRA = 80.f;
    const float PESO_TORRE_CONTESA = 60.f;
    const float PESO_TORRE_NEMICA = 40.f;
    const float PESO_NEMICO_SNIPER = 100.f;
    const float PESO_NEMICO_BRAWLER = 90.f;
    const float PESO_ALTEZZA = 15.f;
    const float PESO_PERICOLO = -50.f;
    const float PESO_DISTANZA_ECCESSO = -5.f;

    int32 AltezzaCella = GrigliaAttore->GetAltezzaCella(CX, CY);
    Score += AltezzaCella * PESO_ALTEZZA;

    for (int32 X = 0; X < GrigliaAttore->GridSize; X++)
        for (int32 Y = 0; Y < GrigliaAttore->GridSize; Y++)
        {
            int32 Idx = X * GrigliaAttore->GridSize + Y;
            if (!GrigliaAttore->Griglia[Idx].bIsTower) continue;
            EStatoTorre Stato = GrigliaAttore->Griglia[Idx].StatoTorre;
            if (Stato == EStatoTorre::ControlloIA) continue;
            float PesoTorre = (Stato == EStatoTorre::Neutrale) ? PESO_TORRE_NEUTRA :
                (Stato == EStatoTorre::Contesa) ? PESO_TORRE_CONTESA : PESO_TORRE_NEMICA;
            int32 DistAttuale = GrigliaAttore->DistanzaManhattan(Unita->PosX, Unita->PosY, X, Y);
            int32 DistNuova = GrigliaAttore->DistanzaManhattan(CX, CY, X, Y);
            Score += (float)(DistAttuale - DistNuova) * PesoTorre / FMath::Max(1, DistNuova + 1);
            if (CX == X && CY == Y) Score += PesoTorre * 2.f;
        }

    for (AUnita* Nemico : UnitaGiocatore)
    {
        if (!Nemico || !Nemico->IsVivo() || Nemico->bSiStaMuovendo) continue;
        int32 DistNemico = GrigliaAttore->DistanzaManhattan(CX, CY, Nemico->PosX, Nemico->PosY);
        if (Unita->TipoUnita == ETipoUnita::Sniper)
        {
            int32 RangeMin = 3, RangeMax = Unita->RangeAttacco - 1;
            if (DistNemico >= RangeMin && DistNemico <= RangeMax) Score += PESO_NEMICO_SNIPER;
            else if (DistNemico < RangeMin) Score += PESO_NEMICO_SNIPER * 0.3f;
            else Score += PESO_NEMICO_SNIPER * (float)Unita->RangeAttacco / FMath::Max(1, DistNemico);
        }
        else
        {
            int32 DistAttuale = GrigliaAttore->DistanzaManhattan(Unita->PosX, Unita->PosY, Nemico->PosX, Nemico->PosY);
            Score += (float)(DistAttuale - DistNemico) * PESO_NEMICO_BRAWLER / FMath::Max(1, DistNemico + 1);
            if (DistNemico == 1) Score += PESO_NEMICO_BRAWLER * 1.5f;
            if (DistNemico > 4) Score += PESO_DISTANZA_ECCESSO * (DistNemico - 4);
        }
        int32 AltezzaNemico = GrigliaAttore->GetAltezzaCella(Nemico->PosX, Nemico->PosY);
        if (DistNemico <= Nemico->RangeAttacco && AltezzaCella <= AltezzaNemico)
            Score += PESO_PERICOLO * (1.5f - (float)Unita->PuntiVita / (float)Unita->PuntiVitaMax);
    }

    if (CX != Unita->PosX || CY != Unita->PosY) Score += 5.f;
    return Score;
}

void AGameManager::TurnoIA()
{
    if (FaseCorrente != EFaseGioco::Gioco || bTurnoGiocatore) return;

    // Log per indicare che l'IA sta usando l'euristica ottimizzata
    UE_LOG(LogTemp, Warning, TEXT("IA sta pensando (euristica ottimizzata)..."));

    bool bAlmenoUnaAzione = false;

    if (GrigliaAttore) GrigliaAttore->PulisciHighlight();

    for (AUnita* Unita : UnitaIA)
    {
        if (!Unita || !Unita->IsVivo() || Unita->bHaAgitoNelTurno || Unita->bSiStaMuovendo) continue;
        bAlmenoUnaAzione = true;

        TArray<AUnita*> NemiciNelRaggio = GetUnitaNemicheNelRaggio(Unita);
        if (NemiciNelRaggio.Num() > 0)
        {
            AUnita* Bersaglio = NemiciNelRaggio[0];
            for (AUnita* N : NemiciNelRaggio)
                if (N && N->IsVivo() && N->PuntiVita < Bersaglio->PuntiVita)
                    Bersaglio = N;

            int32 BersaglioX = Bersaglio->PosX;
            int32 BersaglioY = Bersaglio->PosY;
            int32 UnitaX = Unita->PosX;
            int32 UnitaY = Unita->PosY;

            if (GrigliaAttore)
            {
                GrigliaAttore->CelleAttaccoEvidenziate.Empty();
                GrigliaAttore->CelleAttaccoEvidenziate.Add(FIntPoint(BersaglioX, BersaglioY));
                GrigliaAttore->CellaUnitaSelezionata = FIntPoint(UnitaX, UnitaY);
                GrigliaAttore->bMostraHighlight = true;

                FTimerHandle TimerHL;
                GetWorld()->GetTimerManager().SetTimer(TimerHL, [this]()
                    {
                        if (GrigliaAttore) GrigliaAttore->PulisciHighlight();
                    }, 1.5f, false);
            }

            int32 DannoInflitto = Unita->CalcolaDanno();
            GrigliaAttore->EseguiAttacco(Unita, Bersaglio);
            Unita->bHaAttaccato = true;
            Unita->bHaAgitoNelTurno = true;

            FString CoordBersaglio = GrigliaAttore->GetCoordinateString(BersaglioX, BersaglioY);
            FString TipoStr = (Unita->TipoUnita == ETipoUnita::Sniper) ? TEXT("S") : TEXT("B");
            AggiungiMossaAStorico(FString::Printf(TEXT("AI: %s %s %d"), *TipoStr, *CoordBersaglio, DannoInflitto));
            continue;
        }

        TArray<FIntPoint> CelleRaggiungibili = GrigliaAttore->GetCelleRaggiungibili(
            Unita->PosX, Unita->PosY, Unita->Movimento);
        CelleRaggiungibili.Add(FIntPoint(Unita->PosX, Unita->PosY));

        float ScoreMigliore = -9999.f;
        FIntPoint DestiazioneIdeale(Unita->PosX, Unita->PosY);

        for (const FIntPoint& Cella : CelleRaggiungibili)
        {
            if (IsCellaOccupata(Cella.X, Cella.Y) &&
                !(Cella.X == Unita->PosX && Cella.Y == Unita->PosY)) continue;
            float Score = ValutaCellaIA(Unita, Cella.X, Cella.Y);
            if (Score > ScoreMigliore) { ScoreMigliore = Score; DestiazioneIdeale = Cella; }
        }

        FIntPoint PosFinale(Unita->PosX, Unita->PosY);
        bool bMosso = false;
        TArray<FIntPoint> PercorsoCompleto;

        if (DestiazioneIdeale != PosFinale)
        {
            PercorsoCompleto = GrigliaAttore->GetPercorso(
                Unita->PosX, Unita->PosY,
                DestiazioneIdeale.X, DestiazioneIdeale.Y,
                Unita->Movimento);

            if (PercorsoCompleto.Num() > 0)
            {
                PosFinale = PercorsoCompleto.Last();
                bMosso = true;
            }
        }

        if (bMosso)
        {
            FString CoordDa = GrigliaAttore->GetCoordinateString(Unita->PosX, Unita->PosY);
            FString CoordA = GrigliaAttore->GetCoordinateString(PosFinale.X, PosFinale.Y);
            FString TipoStr = (Unita->TipoUnita == ETipoUnita::Sniper) ? TEXT("S") : TEXT("B");
            AggiungiMossaAStorico(FString::Printf(TEXT("AI: %s %s -> %s"), *TipoStr, *CoordDa, *CoordA));

            if (GrigliaAttore && PercorsoCompleto.Num() > 0)
            {
                for (const FIntPoint& P : PercorsoCompleto)
                    GrigliaAttore->CelleMovimentoEvidenziate.AddUnique(P);
                GrigliaAttore->bMostraHighlight = true;

                FTimerHandle TimerHL;
                GetWorld()->GetTimerManager().SetTimer(TimerHL, [this]()
                    {
                        if (GrigliaAttore) GrigliaAttore->PulisciHighlight();
                    }, 2.0f, false);
            }

            Unita->IniziaMovimentoAnimato(PercorsoCompleto);
            Unita->bHaMosso = true;
        }
        else
        {
            Unita->bHaAgitoNelTurno = true;
        }
    }

    if (!bAlmenoUnaAzione)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA non ha mosse disponibili. Fine turno."));
        FineTurno();
    }
}

bool AGameManager::IsCellaOccupata(int32 X, int32 Y) const
{
    for (AUnita* Unita : UnitaGiocatore)
        if (Unita && Unita->IsVivo() && Unita->PosX == X && Unita->PosY == Y && !Unita->bSiStaMuovendo)
            return true;
    for (AUnita* Unita : UnitaIA)
        if (Unita && Unita->IsVivo() && Unita->PosX == X && Unita->PosY == Y && !Unita->bSiStaMuovendo)
            return true;
    return false;
}

FColor AGameManager::GetColoreUnita(bool bGiocatore) const
{
    return bGiocatore ? ColoreGiocatore : ColoreIA;
}

void AGameManager::ControllaVittoria()
{
    if (TurniControlloGiocatore >= 2)
    {
        FaseCorrente = EFaseGioco::Fine;
        UE_LOG(LogTemp, Warning, TEXT("=== GIOCATORE VINCE per controllo torri! (%d turni) ==="), TurniControlloGiocatore);
        if (WidgetHUD) WidgetHUD->AggiornaHUD();
        return;
    }
    if (TurniControlloIA >= 2)
    {
        FaseCorrente = EFaseGioco::Fine;
        UE_LOG(LogTemp, Warning, TEXT("=== IA VINCE per controllo torri! (%d turni) ==="), TurniControlloIA);
        if (WidgetHUD) WidgetHUD->AggiornaHUD();
        return;
    }
    MostraFaseCorrente();
}

void AGameManager::GestisciClickCella(int32 X, int32 Y)
{
    if (FaseCorrente == EFaseGioco::Fine)
    {
        UE_LOG(LogTemp, Warning, TEXT("Partita terminata."));
        return;
    }

    if (FaseCorrente == EFaseGioco::Posizionamento)
    {
        if (!bTurnoGiocatore)
        {
            UE_LOG(LogTemp, Warning, TEXT("Attendere: e' il turno dell'IA per posizionare."));
            return;
        }
        if (UnitaDaPosizionareGiocatore.Num() == 0) return;
        if (Y < ZonaSchieramentoGiocatoreMin || Y > ZonaSchieramentoGiocatoreMax)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cella (%d,%d) fuori dalla zona di schieramento!"), X, Y);
            return;
        }
        if (!GrigliaAttore || !GrigliaAttore->IsCellaCalpestabile(X, Y)) return;
        if (IsCellaOccupata(X, Y)) return;

        ETipoUnita TipoCorrente = UnitaDaPosizionareGiocatore[0];
        UnitaDaPosizionareGiocatore.RemoveAt(0);
        PosizionaUnita(true, TipoCorrente, X, Y);
        PassaProssimoTurnoPosizionamento();
        return;
    }

    if (FaseCorrente == EFaseGioco::Gioco)
    {
        if (!bTurnoGiocatore)
        {
            UE_LOG(LogTemp, Warning, TEXT("Attendere: e' il turno dell'IA."));
            return;
        }

        if (StatoInputGiocatore == 2 && UnitaSelezionata && UnitaSelezionata->bSiStaMuovendo) return;

        AUnita* UnitaCliccata = nullptr;
        for (AUnita* U : UnitaGiocatore)
        {
            if (U && U->IsVivo() && U->PosX == X && U->PosY == Y && !U->bSiStaMuovendo)
            {
                UnitaCliccata = U;
                break;
            }
        }

        if (StatoInputGiocatore == 0)
        {
            if (UnitaCliccata && !UnitaCliccata->bHaAgitoNelTurno)
            {
                SelezionaUnita(UnitaCliccata);
                StatoInputGiocatore = 1;

                CelleRangeMovimento = GetCelleRaggiungibiliPerUnitaSelezionata();
                if (GrigliaAttore)
                {
                    GrigliaAttore->CelleMovimentoEvidenziate = CelleRangeMovimento;
                    GrigliaAttore->CellaUnitaSelezionata = FIntPoint(UnitaCliccata->PosX, UnitaCliccata->PosY);
                    GrigliaAttore->bMostraHighlight = true;
                }
            }
            else if (UnitaCliccata && UnitaCliccata->bHaAgitoNelTurno)
            {
                UE_LOG(LogTemp, Warning, TEXT("Unita ha gia agito in questo turno."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Nessuna unita del giocatore in (%d,%d). Seleziona una tua unita."), X, Y);
            }
            return;
        }

        if (StatoInputGiocatore == 1 && UnitaSelezionata && !UnitaSelezionata->bSiStaMuovendo)
        {
            if (UnitaCliccata == UnitaSelezionata)
            {
                PulisciEvidenziazioni();
                UnitaSelezionata = nullptr;
                StatoInputGiocatore = 0;
                CelleRangeMovimento.Empty();
                CelleRangeAttacco.Empty();
                return;
            }

            bool bCellaRaggiungibileCheck = CelleRangeMovimento.Contains(FIntPoint(X, Y));
            if (UnitaCliccata && !UnitaCliccata->bHaAgitoNelTurno &&
                !bCellaRaggiungibileCheck && !UnitaCliccata->bSiStaMuovendo)
            {
                PulisciEvidenziazioni();
                SelezionaUnita(UnitaCliccata);
                CelleRangeMovimento = GetCelleRaggiungibiliPerUnitaSelezionata();
                if (GrigliaAttore)
                {
                    GrigliaAttore->CelleMovimentoEvidenziate = CelleRangeMovimento;
                    GrigliaAttore->CellaUnitaSelezionata = FIntPoint(UnitaCliccata->PosX, UnitaCliccata->PosY);
                    GrigliaAttore->bMostraHighlight = true;
                }
                return;
            }

            AUnita* NemicoCliccato = nullptr;
            for (AUnita* U : UnitaIA)
            {
                if (U && U->IsVivo() && U->PosX == X && U->PosY == Y && !U->bSiStaMuovendo)
                {
                    NemicoCliccato = U;
                    break;
                }
            }

            if (NemicoCliccato)
            {
                if (!UnitaSelezionata->bHaAttaccato)
                {
                    TArray<AUnita*> Bersagli = GetUnitaNemicheNelRaggio(UnitaSelezionata);
                    if (Bersagli.Contains(NemicoCliccato))
                    {
                        AttaccaConUnitaSelezionata(NemicoCliccato);
                        return;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Nemico fuori portata!"));
                        return;
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Questa unita ha gia attaccato."));
                    return;
                }
            }

            if (CelleRangeMovimento.Contains(FIntPoint(X, Y)) && !UnitaSelezionata->bHaMosso)
            {
                MuoviUnitaSelezionata(X, Y);
                return;
            }

            UE_LOG(LogTemp, Warning, TEXT("Cella (%d,%d) non raggiungibile. Clicca su una cella evidenziata."), X, Y);
            return;
        }

        if (StatoInputGiocatore == 2)
        {
            if (UnitaSelezionata && UnitaSelezionata->bSiStaMuovendo) return;

            AUnita* NemicoCliccato = nullptr;
            for (AUnita* U : UnitaIA)
            {
                if (U && U->IsVivo() && U->PosX == X && U->PosY == Y && !U->bSiStaMuovendo)
                {
                    NemicoCliccato = U;
                    break;
                }
            }

            if (NemicoCliccato && UnitaSelezionata)
            {
                TArray<AUnita*> Bersagli = GetUnitaNemicheNelRaggio(UnitaSelezionata);
                if (Bersagli.Contains(NemicoCliccato))
                {
                    AttaccaConUnitaSelezionata(NemicoCliccato);
                    return;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Nemico fuori portata!"));
                    return;
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("Azione terminata."));
            if (UnitaSelezionata && !UnitaSelezionata->bHaAttaccato)
                UnitaSelezionata->bHaAgitoNelTurno = true;

            UnitaSelezionata = nullptr;
            StatoInputGiocatore = 0;
            CelleRangeMovimento.Empty();
            CelleRangeAttacco.Empty();
            PulisciEvidenziazioni();
            ControllaFineTurno();
            return;
        }
    }
}

void AGameManager::MostraFaseCorrente()
{
    switch (FaseCorrente)
    {
    case EFaseGioco::LancioMoneta:   UE_LOG(LogTemp, Warning, TEXT("FASE: Lancio Moneta")); break;
    case EFaseGioco::Posizionamento: UE_LOG(LogTemp, Warning, TEXT("FASE: Posizionamento")); break;
    case EFaseGioco::Gioco:          UE_LOG(LogTemp, Warning, TEXT("FASE: Gioco")); break;
    case EFaseGioco::Fine:           UE_LOG(LogTemp, Warning, TEXT("FASE: Fine Partita")); break;
    default: break;
    }
}