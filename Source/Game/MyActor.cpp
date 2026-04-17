#include "MyActor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Unita.h"
#include "GameManager.h"

AMyActor::AMyActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("================================="));
    UE_LOG(LogTemp, Warning, TEXT("=== INIZIO GENERAZIONE GRIGLIA ==="));
    UE_LOG(LogTemp, Warning, TEXT("================================="));

    Griglia.SetNum(GridSize * GridSize);

    GeneraGriglia();
    PosizionaTorri();
    VerificaRaggiungibilita();
    CreaLuceFissa();
    MostraStatistiche();

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle, this, &AMyActor::PosizionaCamera, 0.2f, false);
}

void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DisegnaGriglia();

    if (!bMostraHighlight)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    const float Durata = 0.0f;
    const float SpessoreLinea = 6.0f;
    const float AltezzaOffset = 60.0f;

    auto GetCentro = [&](const FIntPoint& C) -> FVector
        {
            int32 Idx = C.X * GridSize + C.Y;
            float Z = Griglia.IsValidIndex(Idx) ? Griglia[Idx].Z : 0.f;
            return FVector(
                (C.X - GridSize / 2.0f) * CellSize + CellSize / 2,
                (C.Y - GridSize / 2.0f) * CellSize + CellSize / 2,
                Z + AltezzaOffset
            );
        };

    for (const FIntPoint& C : CelleMovimentoEvidenziate)
    {
        FVector Centro = GetCentro(C);
        DrawDebugBox(World, Centro,
            FVector(CellSize / 2 - 8, CellSize / 2 - 8, 4),
            FColor::Cyan, false, Durata, 0, SpessoreLinea);
    }

    for (const FIntPoint& C : CelleAttaccoEvidenziate)
    {
        FVector Centro = GetCentro(C);
        DrawDebugBox(World, Centro,
            FVector(CellSize / 2 - 8, CellSize / 2 - 8, 4),
            FColor::Red, false, Durata, 0, SpessoreLinea);
        DrawDebugLine(World,
            Centro + FVector(-40, -40, 0), Centro + FVector(40, 40, 0),
            FColor::Red, false, Durata, 0, SpessoreLinea);
        DrawDebugLine(World,
            Centro + FVector(-40, 40, 0), Centro + FVector(40, -40, 0),
            FColor::Red, false, Durata, 0, SpessoreLinea);
    }

    if (CellaUnitaSelezionata.X >= 0)
    {
        FVector Centro = GetCentro(CellaUnitaSelezionata);
        DrawDebugBox(World, Centro,
            FVector(CellSize / 2 - 4, CellSize / 2 - 4, 6),
            FColor::White, false, Durata, 0, SpessoreLinea + 2);
    }
}

void AMyActor::CreaLuceFissa()
{
    AActor* ExistingLight = UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass());
    ADirectionalLight* Light = nullptr;

    if (!ExistingLight)
    {
        FActorSpawnParameters SpawnParams;
        Light = GetWorld()->SpawnActor<ADirectionalLight>(
            ADirectionalLight::StaticClass(),
            FVector(0, 0, 5000),
            FRotator(-45, 45, 0),
            SpawnParams);
    }
    else
    {
        Light = Cast<ADirectionalLight>(ExistingLight);
    }

    if (Light)
    {
        Light->SetBrightness(5.0f);
        Light->SetLightColor(FColor::White);
        Light->SetCastShadows(false);
        UE_LOG(LogTemp, Warning, TEXT("Luce fissa creata/configurata"));
    }
}

void AMyActor::GeneraGriglia()
{
    float SeedX = FMath::FRandRange(0.f, 1000.f);
    float SeedY = FMath::FRandRange(0.f, 1000.f);

    UE_LOG(LogTemp, Warning, TEXT("Seed: (%.1f, %.1f)"), SeedX, SeedY);

    int32 TotaleCelle = GridSize * GridSize;
    TArray<float> ValoriNoise;
    ValoriNoise.SetNum(TotaleCelle);

    float ValoreMin = 9999.f;
    float ValoreMax = -9999.f;

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            float Noise1 = FMath::PerlinNoise2D(FVector2D(
                (X + SeedX) * 0.08f, (Y + SeedY) * 0.08f));
            float Noise2 = FMath::PerlinNoise2D(FVector2D(
                (X + SeedX) * 0.25f, (Y + SeedY) * 0.25f));
            float Combinato = Noise1 * 0.7f + Noise2 * 0.3f;
            ValoriNoise[X * GridSize + Y] = Combinato;
            if (Combinato < ValoreMin) ValoreMin = Combinato;
            if (Combinato > ValoreMax) ValoreMax = Combinato;
        }
    }

    float Range = ValoreMax - ValoreMin;
    if (Range < KINDA_SMALL_NUMBER) Range = 1.f;

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            int32 Indice = X * GridSize + Y;
            float Normalizzato = (ValoriNoise[Indice] - ValoreMin) / Range;
            int32 Livello = FMath::FloorToInt(Normalizzato * 5.f);
            Livello = FMath::Clamp(Livello, 0, MaxHeight);

            Griglia[Indice].Altezza = Livello;
            Griglia[Indice].Posizione = FVector2D(
                (X - GridSize / 2.0f) * CellSize,
                (Y - GridSize / 2.0f) * CellSize);
            Griglia[Indice].Z = Livello * 100.f;
            Griglia[Indice].bIsTower = false;
            Griglia[Indice].TowerID = -1;
            Griglia[Indice].StatoTorre = EStatoTorre::Neutrale;
            Griglia[Indice].PuntiTorre = 0;
        }
    }

    int32 Conteggio[5] = { 0,0,0,0,0 };
    for (const FCella& C : Griglia)
        if (C.Altezza >= 0 && C.Altezza <= 4)
            Conteggio[C.Altezza]++;

    for (int32 Livello = 0; Livello <= MaxHeight; Livello++)
    {
        if (Conteggio[Livello] == 0)
        {
            float TargetNorm = (Livello + 0.5f) / 5.f;
            float DistMin = 9999.f;
            int32 IndiceVicino = 0;
            for (int32 i = 0; i < TotaleCelle; i++)
            {
                float Norm = (ValoriNoise[i] - ValoreMin) / Range;
                float Dist = FMath::Abs(Norm - TargetNorm);
                if (Dist < DistMin)
                {
                    DistMin = Dist;
                    IndiceVicino = i;
                }
            }
            Griglia[IndiceVicino].Altezza = Livello;
            Griglia[IndiceVicino].Z = Livello * 100.f;
            UE_LOG(LogTemp, Warning, TEXT("Livello %d forzato sulla cella %d"), Livello, IndiceVicino);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Griglia generata: %d celle"), Griglia.Num());
}

bool AMyActor::VerificaRaggiungibilita()
{
    if (Griglia.Num() == 0) return false;

    FIntPoint Start(-1, -1);
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            if (IsCellaCalpestabile(X, Y))
            {
                Start = FIntPoint(X, Y);
                break;
            }
        }
        if (Start.X >= 0) break;
    }

    if (Start.X < 0) return false;

    TSet<FIntPoint> Visitati;
    TArray<FIntPoint> Frontiera;
    Frontiera.Add(Start);
    Visitati.Add(Start);

    const TArray<FIntPoint> Direzioni = {
        FIntPoint(1,0), FIntPoint(-1,0), FIntPoint(0,1), FIntPoint(0,-1)
    };

    while (Frontiera.Num() > 0)
    {
        FIntPoint Corrente = Frontiera[0];
        Frontiera.RemoveAt(0);

        for (const FIntPoint& Dir : Direzioni)
        {
            int32 NuovoX = Corrente.X + Dir.X;
            int32 NuovoY = Corrente.Y + Dir.Y;
            FIntPoint NuovaPos(NuovoX, NuovoY);

            if (NuovoX >= 0 && NuovoX < GridSize && NuovoY >= 0 && NuovoY < GridSize)
            {
                if (!Visitati.Contains(NuovaPos) && IsCellaCalpestabile(NuovoX, NuovoY))
                {
                    Visitati.Add(NuovaPos);
                    Frontiera.Add(NuovaPos);
                }
            }
        }
    }

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            if (IsCellaCalpestabile(X, Y) && !Visitati.Contains(FIntPoint(X, Y)))
            {
                UE_LOG(LogTemp, Warning, TEXT("Cella (%d,%d) non raggiungibile!"), X, Y);
                return false;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Tutte le celle calpestabili sono raggiungibili"));
    return true;
}

FString AMyActor::GetCoordinateString(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize)
        return FString("INVALIDO");
    TCHAR Lettera = TEXT('A') + X;
    return FString::Printf(TEXT("%c%d"), Lettera, Y);
}

void AMyActor::GetCoordinateDaString(FString CoordString, int32& X, int32& Y) const
{
    X = -1; Y = -1;
    if (CoordString.Len() < 2) return;

    TCHAR Lettera = CoordString[0];
    if (Lettera >= TEXT('A') && Lettera <= TEXT('Y'))
        X = Lettera - TEXT('A');
    else if (Lettera >= TEXT('a') && Lettera <= TEXT('y'))
        X = Lettera - TEXT('a');
    else return;

    FString NumeroStr = CoordString.Right(CoordString.Len() - 1);
    Y = FCString::Atoi(*NumeroStr);
    if (Y < 0 || Y >= GridSize) Y = -1;
}

void AMyActor::PosizionaTorri()
{
    if (!Griglia.IsValidIndex(0)) return;

    TArray<FIntPoint> PosizioniIdeali;
    PosizioniIdeali.Add(FIntPoint(TorreCentraleX, TorreCentraleY));
    PosizioniIdeali.Add(FIntPoint(TorreSinistraX, TorreSinistraY));
    PosizioniIdeali.Add(FIntPoint(TorreDestraX, TorreDestraY));

    UE_LOG(LogTemp, Warning, TEXT("=== POSIZIONAMENTO TORRI ==="));

    for (int32 i = 0; i < PosizioniIdeali.Num(); i++)
    {
        FIntPoint PosIdeale = PosizioniIdeali[i];
        FIntPoint PosizioneFinale = TrovaPosizioneTorreValida(PosIdeale);

        if (PosizioneFinale.X >= 0 && PosizioneFinale.Y >= 0)
        {
            int32 Index = PosizioneFinale.X * GridSize + PosizioneFinale.Y;
            Griglia[Index].bIsTower = true;
            Griglia[Index].TowerID = i + 1;
            Griglia[Index].StatoTorre = EStatoTorre::Neutrale;
            UE_LOG(LogTemp, Warning, TEXT("Torre %d posizionata a (%d,%d) (ideale era %d,%d)"),
                i + 1, PosizioneFinale.X, PosizioneFinale.Y, PosIdeale.X, PosIdeale.Y);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Impossibile posizionare torre %d vicino a (%d,%d)!"),
                i + 1, PosIdeale.X, PosIdeale.Y);
        }
    }
}

FIntPoint AMyActor::TrovaPosizioneTorreValida(FIntPoint PosizioneIdeale)
{
    if (IsCellaValidaPerTorre(PosizioneIdeale.X, PosizioneIdeale.Y))
        return PosizioneIdeale;

    for (int32 Raggio = 1; Raggio <= 5; Raggio++)
    {
        for (int32 DX = -Raggio; DX <= Raggio; DX++)
        {
            for (int32 DY = -Raggio; DY <= Raggio; DY++)
            {
                if (FMath::Abs(DX) == Raggio || FMath::Abs(DY) == Raggio)
                {
                    int32 NuovoX = PosizioneIdeale.X + DX;
                    int32 NuovoY = PosizioneIdeale.Y + DY;
                    if (IsCellaValidaPerTorre(NuovoX, NuovoY))
                        return FIntPoint(NuovoX, NuovoY);
                }
            }
        }
    }

    return FIntPoint(-1, -1);
}

bool AMyActor::IsCellaValidaPerTorre(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return false;
    int32 Index = X * GridSize + Y;
    if (!Griglia.IsValidIndex(Index)) return false;
    if (Griglia[Index].bIsTower) return false;
    if (Griglia[Index].Altezza == 0) return false;
    return true;
}

void AMyActor::PosizionaCamera()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController non trovato, riprovo tra 0.1s"));
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle, this, &AMyActor::PosizionaCamera, 0.1f, false);
        return;
    }

    float DimensioneTotale = GridSize * CellSize;
    float AltezzaCamera = DimensioneTotale * 1.2f;

    PlayerController->GetPawn()->SetActorLocationAndRotation(
        FVector(0.f, 0.f, AltezzaCamera),
        FRotator(-90.f, 0.f, 0.f));

    UE_LOG(LogTemp, Warning, TEXT("Camera posizionata a (0,0,%.2f)"), AltezzaCamera);
}

// ========== FUNZIONI PUBBLICHE ==========

TArray<FIntPoint> AMyActor::GetCelleCalpestabili() const
{
    TArray<FIntPoint> Celle;
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            int32 Index = X * GridSize + Y;
            if (Griglia[Index].Altezza > 0 && !Griglia[Index].bIsTower)
                Celle.Add(FIntPoint(X, Y));
        }
    }
    return Celle;
}

bool AMyActor::IsCellaCalpestabile(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return false;
    int32 Index = X * GridSize + Y;
    return (Griglia[Index].Altezza > 0 && !Griglia[Index].bIsTower);
}

int32 AMyActor::GetAltezzaCella(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return -1;
    return Griglia[X * GridSize + Y].Altezza;
}

bool AMyActor::IsTower(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return false;
    return Griglia[X * GridSize + Y].bIsTower;
}

FVector AMyActor::GetPosizioneMondo(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return FVector::ZeroVector;
    int32 Index = X * GridSize + Y;
    return FVector(
        (X - GridSize / 2.0f) * CellSize + CellSize / 2,
        (Y - GridSize / 2.0f) * CellSize + CellSize / 2,
        Griglia[Index].Z + 100.f);
}

// ========== FUNZIONI MOVIMENTO ==========

int32 AMyActor::CalcolaCostoMovimento(int32 DaX, int32 DaY, int32 AX, int32 AY) const
{
    if (!IsMovimentoValido(DaX, DaY, AX, AY)) return -1;
    int32 AltezzaPartenza = Griglia[DaX * GridSize + DaY].Altezza;
    int32 AltezzaDestinazione = Griglia[AX * GridSize + AY].Altezza;
    return (AltezzaDestinazione > AltezzaPartenza) ? 2 : 1;
}

bool AMyActor::IsMovimentoValido(int32 DaX, int32 DaY, int32 AX, int32 AY) const
{
    if (DaX < 0 || DaX >= GridSize || DaY < 0 || DaY >= GridSize ||
        AX < 0 || AX >= GridSize || AY < 0 || AY >= GridSize)
        return false;
    if (!IsCellaCalpestabile(DaX, DaY) || !IsCellaCalpestabile(AX, AY))
        return false;
    if (FMath::Abs(DaX - AX) + FMath::Abs(DaY - AY) != 1)
        return false;
    return true;
}

TArray<FIntPoint> AMyActor::GetCelleRaggiungibili(int32 StartX, int32 StartY, int32 MovimentoMax) const
{
    TArray<FIntPoint> CelleRaggiungibili;
    TMap<FIntPoint, int32> CostoMinimo;
    TArray<TPair<FIntPoint, int32>> Frontiera;

    FIntPoint Start(StartX, StartY);
    Frontiera.Add(TPair<FIntPoint, int32>(Start, 0));
    CostoMinimo.Add(Start, 0);

    const TArray<FIntPoint> Direzioni = {
        FIntPoint(1,0), FIntPoint(-1,0), FIntPoint(0,1), FIntPoint(0,-1)
    };

    while (Frontiera.Num() > 0)
    {
        TPair<FIntPoint, int32> Corrente = Frontiera[0];
        Frontiera.RemoveAt(0);

        FIntPoint Pos = Corrente.Key;
        int32 CostoCorrente = Corrente.Value;

        if (CostoMinimo.Contains(Pos) && CostoMinimo[Pos] < CostoCorrente)
            continue;

        for (const FIntPoint& Dir : Direzioni)
        {
            int32 NuovoX = Pos.X + Dir.X;
            int32 NuovoY = Pos.Y + Dir.Y;
            FIntPoint NuovaPos(NuovoX, NuovoY);

            if (NuovoX < 0 || NuovoX >= GridSize || NuovoY < 0 || NuovoY >= GridSize)
                continue;

            int32 CostoSpostamento = CalcolaCostoMovimento(Pos.X, Pos.Y, NuovoX, NuovoY);
            if (CostoSpostamento == -1)
                continue;

            int32 NuovoCosto = CostoCorrente + CostoSpostamento;
            if (NuovoCosto <= MovimentoMax)
            {
                if (!CostoMinimo.Contains(NuovaPos) || CostoMinimo[NuovaPos] > NuovoCosto)
                {
                    CostoMinimo.Add(NuovaPos, NuovoCosto);
                    Frontiera.Add(TPair<FIntPoint, int32>(NuovaPos, NuovoCosto));
                }
            }
        }
    }

    for (auto& Pair : CostoMinimo)
        if (Pair.Key != Start)
            CelleRaggiungibili.Add(Pair.Key);

    return CelleRaggiungibili;
}

TArray<FIntPoint> AMyActor::GetPercorso(int32 DaX, int32 DaY, int32 AX, int32 AY, int32 MovimentoMax) const
{
    TArray<FIntPoint> Percorso;
    if (!GetWorld())
        return Percorso;

    FIntPoint Start(DaX, DaY);
    FIntPoint Destinazione(AX, AY);

    TMap<FIntPoint, int32> G;
    TMap<FIntPoint, FIntPoint> Padre;
    TArray<TPair<int32, FIntPoint>> OpenList;

    auto Heuristica = [&](FIntPoint A, FIntPoint B) -> int32
        {
            return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
        };

    auto InserisciOrdinato = [&](int32 F, FIntPoint Nodo)
        {
            int32 i = 0;
            while (i < OpenList.Num() && OpenList[i].Key <= F)
                i++;
            OpenList.Insert(TPair<int32, FIntPoint>(F, Nodo), i);
        };

    G.Add(Start, 0);
    Padre.Add(Start, FIntPoint(-1, -1));
    InserisciOrdinato(Heuristica(Start, Destinazione), Start);

    TSet<FIntPoint> Chiusi;
    bool bTrovato = false;

    const TArray<FIntPoint> Direzioni = {
        FIntPoint(1,0), FIntPoint(-1,0), FIntPoint(0,1), FIntPoint(0,-1)
    };

    while (OpenList.Num() > 0)
    {
        TPair<int32, FIntPoint> Corrente = OpenList[0];
        OpenList.RemoveAt(0);

        FIntPoint Pos = Corrente.Value;
        if (Chiusi.Contains(Pos))
            continue;
        Chiusi.Add(Pos);

        if (Pos == Destinazione)
        {
            bTrovato = true;
            break;
        }

        int32 GCorrente = G.Contains(Pos) ? G[Pos] : 9999;

        for (const FIntPoint& Dir : Direzioni)
        {
            int32 NuovoX = Pos.X + Dir.X;
            int32 NuovoY = Pos.Y + Dir.Y;
            FIntPoint Vicino(NuovoX, NuovoY);

            if (NuovoX < 0 || NuovoX >= GridSize || NuovoY < 0 || NuovoY >= GridSize)
                continue;
            if (Chiusi.Contains(Vicino))
                continue;

            int32 CostoMossa = CalcolaCostoMovimento(Pos.X, Pos.Y, NuovoX, NuovoY);
            if (CostoMossa == -1)
                continue;

            int32 NuovoG = GCorrente + CostoMossa;
            if (!G.Contains(Vicino) || NuovoG < G[Vicino])
            {
                G.Add(Vicino, NuovoG);
                Padre.Add(Vicino, Pos);
                InserisciOrdinato(NuovoG + Heuristica(Vicino, Destinazione), Vicino);
            }
        }
    }

    if (bTrovato)
    {
        FIntPoint Curr = Destinazione;
        while (Curr != Start && Padre.Contains(Curr))
        {
            Percorso.Insert(Curr, 0);
            Curr = Padre[Curr];
        }

        int32 CostoAccumulato = 0;
        TArray<FIntPoint> PercorsoFiltrato;
        FIntPoint Precedente = Start;
        for (const FIntPoint& Passo : Percorso)
        {
            int32 CostoPasso = CalcolaCostoMovimento(Precedente.X, Precedente.Y, Passo.X, Passo.Y);
            CostoAccumulato += CostoPasso;
            if (CostoAccumulato > MovimentoMax)
                break;
            PercorsoFiltrato.Add(Passo);
            Precedente = Passo;
        }
        return PercorsoFiltrato;
    }
    else
    {
        int32 MiglioreDist = 9999;
        FIntPoint NodoMigliore = Start;

        for (const FIntPoint& Nodo : Chiusi)
        {
            if (!G.Contains(Nodo) || G[Nodo] > MovimentoMax)
                continue;
            int32 Dist = Heuristica(Nodo, Destinazione);
            if (Dist < MiglioreDist)
            {
                MiglioreDist = Dist;
                NodoMigliore = Nodo;
            }
        }

        if (NodoMigliore != Start && Padre.Contains(NodoMigliore))
        {
            FIntPoint Curr = NodoMigliore;
            while (Curr != Start && Padre.Contains(Curr))
            {
                Percorso.Insert(Curr, 0);
                Curr = Padre[Curr];
            }
        }
        return Percorso;
    }
}

void AMyActor::DisegnaCelleRaggiungibili(const TArray<FIntPoint>& Celle)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FIntPoint& Cella : Celle)
    {
        int32 Index = Cella.X * GridSize + Cella.Y;
        FVector Centro(
            (Cella.X - GridSize / 2.0f) * CellSize + CellSize / 2,
            (Cella.Y - GridSize / 2.0f) * CellSize + CellSize / 2,
            Griglia[Index].Z + 50);

        DrawDebugCircle(World, Centro, CellSize / 3, 24, FColor::Cyan,
            false, 30.0f, 0, 8, FVector(1, 0, 0), FVector(0, 1, 0), true);
        DrawDebugBox(World, Centro,
            FVector(CellSize / 2 - 10, CellSize / 2 - 10, 5),
            FColor::Blue, false, 30.0f, 0, 5);
    }
}

// ========== FUNZIONI ATTACCO ==========

int32 AMyActor::DistanzaManhattan(int32 X1, int32 Y1, int32 X2, int32 Y2) const
{
    return FMath::Abs(X1 - X2) + FMath::Abs(Y1 - Y2);
}

TArray<FIntPoint> AMyActor::GetLineaBresenham(int32 X1, int32 Y1, int32 X2, int32 Y2) const
{
    TArray<FIntPoint> Linea;
    int32 dx = FMath::Abs(X2 - X1), dy = FMath::Abs(Y2 - Y1);
    int32 sx = (X1 < X2) ? 1 : -1, sy = (Y1 < Y2) ? 1 : -1;
    int32 err = dx - dy, x = X1, y = Y1;

    while (true)
    {
        Linea.Add(FIntPoint(x, y));
        if (x == X2 && y == Y2) break;
        int32 e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
    return Linea;
}

bool AMyActor::PuoAttaccare(int32 AttaccanteX, int32 AttaccanteY, int32 AttaccanteLivello,
    int32 DifensoreX, int32 DifensoreY, int32 DifensoreLivello,
    ETipoAttacco Tipo, int32 Range) const
{
    if (AttaccanteX < 0 || AttaccanteX >= GridSize || AttaccanteY < 0 || AttaccanteY >= GridSize ||
        DifensoreX < 0 || DifensoreX >= GridSize || DifensoreY < 0 || DifensoreY >= GridSize)
        return false;

    if (DistanzaManhattan(AttaccanteX, AttaccanteY, DifensoreX, DifensoreY) > Range)
        return false;
    if (DifensoreLivello > AttaccanteLivello)
        return false;
    if (Tipo == ETipoAttacco::Distanza)
        return true;

    if (Tipo == ETipoAttacco::CortoRaggio)
    {
        TArray<FIntPoint> Linea = GetLineaBresenham(AttaccanteX, AttaccanteY, DifensoreX, DifensoreY);
        for (int32 i = 1; i < Linea.Num() - 1; i++)
        {
            int32 Index = Linea[i].X * GridSize + Linea[i].Y;
            if (!Griglia.IsValidIndex(Index)) continue;
            if (Griglia[Index].Altezza == 0 || Griglia[Index].bIsTower)
                return false;
        }
    }
    return true;
}

TArray<FIntPoint> AMyActor::GetUnitaNelRaggio(int32 AttaccanteX, int32 AttaccanteY,
    ETipoAttacco Tipo, int32 Range) const
{
    TArray<FIntPoint> UnitaNelRaggio;

    int32 MinX = FMath::Max(0, AttaccanteX - Range);
    int32 MaxX = FMath::Min(GridSize - 1, AttaccanteX + Range);
    int32 MinY = FMath::Max(0, AttaccanteY - Range);
    int32 MaxY = FMath::Min(GridSize - 1, AttaccanteY + Range);

    for (int32 X = MinX; X <= MaxX; X++)
    {
        for (int32 Y = MinY; Y <= MaxY; Y++)
        {
            if (X == AttaccanteX && Y == AttaccanteY)
                continue;
            if (DistanzaManhattan(AttaccanteX, AttaccanteY, X, Y) <= Range)
                UnitaNelRaggio.Add(FIntPoint(X, Y));
        }
    }

    return UnitaNelRaggio;
}

bool AMyActor::EseguiAttacco(AUnita* Attaccante, AUnita* Difensore)
{
    if (!Attaccante || !Difensore)
        return false;
    if (!Attaccante->IsVivo() || !Difensore->IsVivo())
        return false;

    int32 IndexA = Attaccante->PosX * GridSize + Attaccante->PosY;
    int32 IndexD = Difensore->PosX * GridSize + Difensore->PosY;

    if (!Griglia.IsValidIndex(IndexA) || !Griglia.IsValidIndex(IndexD))
        return false;

    int32 LivelloAttaccante = Griglia[IndexA].Altezza;
    int32 LivelloDifensore = Griglia[IndexD].Altezza;

    if (!PuoAttaccare(Attaccante->PosX, Attaccante->PosY, LivelloAttaccante,
        Difensore->PosX, Difensore->PosY, LivelloDifensore,
        Attaccante->TipoAttacco, Attaccante->RangeAttacco))
    {
        UE_LOG(LogTemp, Warning, TEXT("Attacco non valido!"));
        return false;
    }

    int32 DannoReale = Attaccante->CalcolaDanno();
    Difensore->PrendiDanno(DannoReale);

    int32 Distanza = DistanzaManhattan(Attaccante->PosX, Attaccante->PosY,
        Difensore->PosX, Difensore->PosY);

    AGameManager* GameManager = Cast<AGameManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

    if (GameManager)
    {
        int32 DannoContrattacco = GameManager->CalcolaDannoContrattacco(Attaccante, Difensore, Distanza);
        if (DannoContrattacco > 0)
            Attaccante->PrendiDanno(DannoContrattacco);
    }

    FString Mossa = Attaccante->bIsGiocatore ? TEXT("HP: ") : TEXT("AI: ");
    Mossa += (Attaccante->TipoUnita == ETipoUnita::Sniper) ? TEXT("S ") : TEXT("B ");
    Mossa += FString::Printf(TEXT("%s %d"),
        *GetCoordinateString(Difensore->PosX, Difensore->PosY), DannoReale);

    if (GameManager)
        GameManager->AggiungiMossaAStorico(Mossa);

    UE_LOG(LogTemp, Warning, TEXT("ATTACCO! %s (%s) infligge %d danni a %s (%s) - Vita rimasta: %d/%d"),
        Attaccante->bIsGiocatore ? TEXT("Giocatore") : TEXT("IA"),
        *Attaccante->GetTipoUnitaAsString(), DannoReale,
        Difensore->bIsGiocatore ? TEXT("Giocatore") : TEXT("IA"),
        *Difensore->GetTipoUnitaAsString(),
        Difensore->PuntiVita, Difensore->PuntiVitaMax);

    return true;
}

// ========== FUNZIONI DISEGNO ==========

FColor AMyActor::GetColoreCella(int32 Altezza, bool bIsTower, EStatoTorre Stato) const
{
    if (bIsTower)
    {
        switch (Stato)
        {
        case EStatoTorre::Neutrale:    return FColor(200, 200, 200);
        case EStatoTorre::ControlloP1: return FColor(0, 255, 255);
        case EStatoTorre::ControlloIA: return FColor(255, 0, 255);
        case EStatoTorre::Contesa:
        {
            float Time = GetWorld()->GetTimeSeconds();
            return (FMath::FloorToInt(Time * 2) % 2 == 0)
                ? FColor(0, 255, 255) : FColor(255, 0, 255);
        }
        default: return FColor(200, 200, 200);
        }
    }

    switch (Altezza)
    {
    case 0:  return FColor(0, 0, 255);
    case 1:  return FColor(0, 255, 0);
    case 2:  return FColor(255, 255, 0);
    case 3:  return FColor(255, 128, 0);
    case 4:  return FColor(255, 0, 0);
    default: return FColor(255, 255, 255);
    }
}

void AMyActor::DisegnaGriglia()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            int32 Index = X * GridSize + Y;
            const FCella& Cella = Griglia[Index];

            FVector TL(Cella.Posizione.X, Cella.Posizione.Y, Cella.Z);
            FVector TR(Cella.Posizione.X + CellSize, Cella.Posizione.Y, Cella.Z);
            FVector BL(Cella.Posizione.X, Cella.Posizione.Y + CellSize, Cella.Z);
            FVector BR(Cella.Posizione.X + CellSize, Cella.Posizione.Y + CellSize, Cella.Z);
            FVector Centro = FVector(
                Cella.Posizione.X + CellSize / 2,
                Cella.Posizione.Y + CellSize / 2,
                Cella.Z);

            DrawDebugSolidBox(World, Centro, FVector(CellSize / 2, CellSize / 2, 20),
                FQuat::Identity, GetColoreCella(Cella.Altezza, false, EStatoTorre::Neutrale), false, -1, 0);

            if (Cella.bIsTower)
            {
                float AltezzaBase = Cella.Z;

                // *** FIX: ColoreTorre cambia in base allo stato — colora TUTTA la torre ***
                FColor ColoreTorre;
                FColor ColoreGemma;

                switch (Cella.StatoTorre)
                {
                case EStatoTorre::Neutrale:
                    ColoreTorre = FColor(120, 85, 55);      // marrone neutrale
                    ColoreGemma = FColor(200, 200, 200);    // grigio
                    break;

                case EStatoTorre::ControlloP1:
                    ColoreTorre = FColor(0, 180, 180);      // ciano scuro (giocatore)
                    ColoreGemma = FColor(0, 255, 255);      // ciano brillante
                    break;

                case EStatoTorre::ControlloIA:
                    ColoreTorre = FColor(180, 0, 180);      // viola scuro (IA)
                    ColoreGemma = FColor(255, 0, 255);      // magenta brillante
                    break;

                case EStatoTorre::Contesa:
                {
                    float T = GetWorld()->GetTimeSeconds();
                    bool bToggle = (FMath::FloorToInt(T * 3) % 2 == 0);
                    ColoreTorre = bToggle ? FColor(0, 180, 180) : FColor(180, 0, 180);
                    ColoreGemma = bToggle ? FColor(255, 215, 0) : FColor(255, 140, 0);
                    break;
                }

                default:
                    ColoreTorre = FColor(120, 85, 55);
                    ColoreGemma = FColor(200, 200, 200);
                    break;
                }

                // ColoreScuro = 60% del ColoreTorre per fasce e merli
                FColor ColoreScuro = FColor(
                    FMath::Clamp((int32)(ColoreTorre.R * 0.6f), 0, 255),
                    FMath::Clamp((int32)(ColoreTorre.G * 0.6f), 0, 255),
                    FMath::Clamp((int32)(ColoreTorre.B * 0.6f), 0, 255)
                );

                // Base
                DrawDebugSolidBox(World, FVector(Centro.X, Centro.Y, AltezzaBase + 40),
                    FVector(140, 140, 80), FQuat::Identity, ColoreScuro, false, -1, 0);

                // Corpo principale (ora usa ColoreTorre)
                DrawDebugSolidBox(World, FVector(Centro.X, Centro.Y, AltezzaBase + 260),
                    FVector(110, 110, 360), FQuat::Identity, ColoreTorre, false, -1, 0);

                // Fasce decorative (3 anelli, usa ColoreScuro)
                for (int32 i = 0; i < 3; i++)
                {
                    float YFascia = AltezzaBase + 120 + i * 100;
                    DrawDebugSolidBox(World, FVector(Centro.X, Centro.Y, YFascia),
                        FVector(120, 120, 20), FQuat::Identity, ColoreScuro, false, -1, 0);
                }

                // Merli (usa ColoreScuro)
                float YMerlo = AltezzaBase + 440;
                for (int32 i = -1; i <= 1; i++)
                {
                    for (int32 j = -1; j <= 1; j++)
                    {
                        if (i == 0 && j == 0) continue;
                        DrawDebugSolidBox(World,
                            FVector(Centro.X + i * 60, Centro.Y + j * 60, YMerlo),
                            FVector(35, 35, 50), FQuat::Identity, ColoreScuro, false, -1, 0);
                    }
                }

                // Gemma in cima (ColoreGemma brillante)
                DrawDebugSphere(World, FVector(Centro.X, Centro.Y, AltezzaBase + 490),
                    40, 16, ColoreGemma, false, -1, 0, 6);

                // Puntale (sempre bianco/oro)
                DrawDebugSphere(World, FVector(Centro.X, Centro.Y, AltezzaBase + 530),
                    18, 12, FColor(255, 240, 200), false, -1, 0, 4);
            }

            // Bordi cella
            DrawDebugLine(World, TL, TR, FColor::Black, false, -1, 0, 2);
            DrawDebugLine(World, TR, BR, FColor::Black, false, -1, 0, 2);
            DrawDebugLine(World, BR, BL, FColor::Black, false, -1, 0, 2);
            DrawDebugLine(World, BL, TL, FColor::Black, false, -1, 0, 2);
        }
    }
}

void AMyActor::MostraStatistiche()
{
    int32 ConteggioAltezze[5] = { 0,0,0,0,0 };
    int32 ConteggioTorri = 0;

    for (const FCella& Cella : Griglia)
    {
        if (Cella.bIsTower)
            ConteggioTorri++;
        else if (Cella.Altezza >= 0 && Cella.Altezza <= 4)
            ConteggioAltezze[Cella.Altezza]++;
    }

    UE_LOG(LogTemp, Warning, TEXT("================================="));
    UE_LOG(LogTemp, Warning, TEXT("STATISTICHE:"));
    UE_LOG(LogTemp, Warning, TEXT("  Blu    (0): %d celle"), ConteggioAltezze[0]);
    UE_LOG(LogTemp, Warning, TEXT("  Verde  (1): %d celle"), ConteggioAltezze[1]);
    UE_LOG(LogTemp, Warning, TEXT("  Giallo (2): %d celle"), ConteggioAltezze[2]);
    UE_LOG(LogTemp, Warning, TEXT("  Arancio(3): %d celle"), ConteggioAltezze[3]);
    UE_LOG(LogTemp, Warning, TEXT("  Rosso  (4): %d celle"), ConteggioAltezze[4]);
    UE_LOG(LogTemp, Warning, TEXT("  Torri:      %d"), ConteggioTorri);
    UE_LOG(LogTemp, Warning, TEXT("================================="));
}

void AMyActor::PulisciHighlight()
{
    CelleMovimentoEvidenziate.Empty();
    CelleAttaccoEvidenziate.Empty();
    CellaUnitaSelezionata = FIntPoint(-1, -1);
    bMostraHighlight = false;
}