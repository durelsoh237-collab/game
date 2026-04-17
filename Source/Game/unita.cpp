#include "Unita.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MyActor.h"
#include "GameManager.h"
#include "DrawDebugHelpers.h"

AUnita::AUnita()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = Root;

    bIsBeingDestroyed = false;
    bSiStaMuovendo = false;
    IndicePercorsoCorrente = 0;
    TempoTrascorsoPerCella = 0.f;
    TempoPerCella = 0.3f;

    GameManager = nullptr;
}

void AUnita::BeginPlay()
{
    Super::BeginPlay();
    PuntiVita = PuntiVitaMax;
    ImpostaDannoPerTipo();
    ResetTurno();

    if (!RootComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Unita %s senza RootComponent!"), *GetName());
    }
}

void AUnita::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bSiStaMuovendo)
    {
        AggiornaMovimento(DeltaTime);
    }

    DisegnaUnita();
}

void AUnita::ResetTurno()
{
    bHaMosso = false;
    bHaAttaccato = false;
    bHaAgitoNelTurno = false;
}

void AUnita::ImpostaDannoPerTipo()
{
    switch (TipoUnita)
    {
    case ETipoUnita::Sniper:
        Movimento = 4;
        TipoAttacco = ETipoAttacco::Distanza;
        RangeAttacco = 10;
        DannoMin = 4;
        DannoMax = 8;
        PuntiVita = 20;
        PuntiVitaMax = 20;
        break;

    case ETipoUnita::Brawler:
        Movimento = 6;
        TipoAttacco = ETipoAttacco::CortoRaggio;
        RangeAttacco = 1;
        DannoMin = 1;
        DannoMax = 6;
        PuntiVita = 40;
        PuntiVitaMax = 40;
        break;

    default:
        Movimento = 3;
        TipoAttacco = ETipoAttacco::CortoRaggio;
        RangeAttacco = 1;
        DannoMin = 1;
        DannoMax = 3;
        PuntiVita = 10;
        PuntiVitaMax = 10;
        break;
    }
}

int32 AUnita::CalcolaDanno() const
{
    if (DannoMax <= DannoMin)
        return DannoMin;
    return FMath::RandRange(DannoMin, DannoMax);
}

bool AUnita::IsVivo() const
{
    return PuntiVita > 0 && !bIsBeingDestroyed;
}

void AUnita::PrendiDanno(int32 DannoSubito)
{
    if (!IsVivo() || bIsBeingDestroyed)
        return;

    PuntiVita -= DannoSubito;

    if (PuntiVita <= 0)
    {
        PuntiVita = 0;
        bIsBeingDestroyed = true;
        UE_LOG(LogTemp, Warning, TEXT("Unita %s distrutta! Verra respawnata..."), *GetName());
        Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unita %s ha %d/%d PV"), *GetName(), PuntiVita, PuntiVitaMax);
    }
}

void AUnita::SetPosizione(int32 X, int32 Y)
{
    if (bIsBeingDestroyed)
        return;

    PosX = X;
    PosY = Y;

    AMyActor* MyActor = Cast<AMyActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyActor::StaticClass()));

    if (!MyActor)
    {
        UE_LOG(LogTemp, Error, TEXT("MyActor non trovato in SetPosizione!"));
        return;
    }

    float CellSize = MyActor->CellSize;
    float GridSize = MyActor->GridSize;
    float MetaGrid = GridSize / 2.0f;

    int32 Index = X * (int32)GridSize + Y;
    float AltezzaCella = 0.f;

    if (MyActor->Griglia.IsValidIndex(Index))
        AltezzaCella = MyActor->Griglia[Index].Z;

    FVector NuovaPosizione(
        (X - MetaGrid) * CellSize + CellSize / 2,
        (Y - MetaGrid) * CellSize + CellSize / 2,
        AltezzaCella + 225.0f
    );

    SetActorLocation(NuovaPosizione);
}

void AUnita::DisegnaUnita()
{
    if (bIsBeingDestroyed)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    FVector Posizione = GetActorLocation();

    if (Posizione.IsNearlyZero())
        return;

    // Cerchio base - dimensione proporzionata alla cella (cella poco più grande)
    DrawDebugCircle(World,
        FVector(Posizione.X, Posizione.Y, Posizione.Z - 60.f),
        95, 48,
        ColoreUnita,
        false, -1, 0, 15,
        FVector(1, 0, 0),
        FVector(0, 1, 0),
        true);

    // ===== COLORE SFERA: STESSO COLORE PER ENTRAMBE LE UNITÀ DELLO STESSO GIOCATORE =====
    FColor ColoreTipo;
    if (bIsGiocatore)
    {
        ColoreTipo = FColor::Cyan;      // GIOCATORE: Ciano (sia Sniper che Brawler)
    }
    else
    {
        ColoreTipo = FColor::Magenta;   // IA: Magenta (sia Sniper che Brawler)
    }
    // ===== FINE MODIFICA =====

    // Sfera centrale - dimensione proporzionata alla cella (cella poco più grande)
    DrawDebugSphere(World,
        FVector(Posizione.X, Posizione.Y, Posizione.Z),
        85, 24,
        ColoreTipo,
        false, -1, 0, 8);

    // Barra vita
    float PercentualeVita = (PuntiVitaMax > 0) ? (float)PuntiVita / (float)PuntiVitaMax : 0.f;
    int32 LunghezzaBarra = 150;
    int32 AltezzaBarra = 20;

    FColor ColoreVita = FColor::Green;
    if (PercentualeVita < 0.6f) ColoreVita = FColor::Yellow;
    if (PercentualeVita < 0.3f) ColoreVita = FColor::Red;

    // Sfondo barra vita
    DrawDebugSolidBox(World,
        FVector(Posizione.X, Posizione.Y, Posizione.Z + 165),
        FVector(LunghezzaBarra, 10, AltezzaBarra),
        FQuat::Identity, FColor::Black,
        false, -1, 0);

    // Barra vita colorata
    DrawDebugSolidBox(World,
        FVector(
            Posizione.X - (LunghezzaBarra * (1.f - PercentualeVita) / 2.f),
            Posizione.Y,
            Posizione.Z + 165),
        FVector(LunghezzaBarra * PercentualeVita, 10, AltezzaBarra),
        FQuat::Identity, ColoreVita,
        false, -1, 0);

    // Etichetta
    FString Etichetta = FString::Printf(TEXT("%s\n%s (%d,%d)"),
        bIsGiocatore ? TEXT("GIOCATORE") : TEXT("IA"),
        *GetTipoUnitaAsString(),
        PosX, PosY);

    DrawDebugString(World,
        FVector(Posizione.X - 100, Posizione.Y - 50, Posizione.Z + 263),
        Etichetta,
        nullptr, FColor::White,
        0.0f, true, 180.f);

    FString VitaStr = FString::Printf(TEXT("%d/%d"), PuntiVita, PuntiVitaMax);
    DrawDebugString(World,
        FVector(Posizione.X - 50, Posizione.Y + 30, Posizione.Z + 210),
        VitaStr,
        nullptr, ColoreVita,
        0.0f, true, 140.f);
}

FString AUnita::GetTipoAttaccoAsString() const
{
    switch (TipoAttacco)
    {
    case ETipoAttacco::CortoRaggio: return FString("Corto Raggio");
    case ETipoAttacco::Distanza:    return FString("Distanza");
    default:                        return FString("Sconosciuto");
    }
}

FString AUnita::GetTipoUnitaAsString() const
{
    switch (TipoUnita)
    {
    case ETipoUnita::Sniper:  return FString("Sniper");
    case ETipoUnita::Brawler: return FString("Brawler");
    default:                  return FString("Sconosciuto");
    }
}

void AUnita::ResettaVita()
{
    if (!bIsBeingDestroyed)
        PuntiVita = PuntiVitaMax;
}

void AUnita::IniziaMovimentoAnimato(const TArray<FIntPoint>& Percorso)
{
    if (Percorso.Num() == 0)
    {
        bSiStaMuovendo = false;
        return;
    }

    PercorsoMovimento = Percorso;
    IndicePercorsoCorrente = 0;
    TempoTrascorsoPerCella = 0.f;
    bSiStaMuovendo = true;
}

void AUnita::AggiornaMovimento(float DeltaTime)
{
    if (!bSiStaMuovendo || !PercorsoMovimento.IsValidIndex(IndicePercorsoCorrente))
    {
        bSiStaMuovendo = false;
        return;
    }

    TempoTrascorsoPerCella += DeltaTime;

    if (TempoTrascorsoPerCella >= TempoPerCella)
    {
        ProssimaCella();
        TempoTrascorsoPerCella = 0.f;
    }
}

void AUnita::ProssimaCella()
{
    IndicePercorsoCorrente++;

    if (IndicePercorsoCorrente >= PercorsoMovimento.Num())
    {
        bSiStaMuovendo = false;

        if (PercorsoMovimento.Num() > 0)
        {
            FIntPoint DestinazioneFinale = PercorsoMovimento.Last();
            SetPosizione(DestinazioneFinale.X, DestinazioneFinale.Y);
        }

        bHaAgitoNelTurno = true;
        PercorsoMovimento.Empty();

        if (GameManager)
            GameManager->ControllaFineTurno();
    }
    else
    {
        FIntPoint ProssimaCellaPos = PercorsoMovimento[IndicePercorsoCorrente];
        SetPosizione(ProssimaCellaPos.X, ProssimaCellaPos.Y);
    }
}