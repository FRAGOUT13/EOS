#include "EOSLoginWidget.h"

#include "EOSLoginSample.h"
#include "EOSLobbySubsystem.h"
#include "Engine/GameInstance.h"

void UEOSLoginWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindToSubsystem();
}

void UEOSLoginWidget::NativeDestruct()
{
    if (LobbySubsystem)
    {
        LobbySubsystem->OnLoginComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleLoginComplete);
        LobbySubsystem->OnCreateSessionComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleCreateLobby);
        LobbySubsystem->OnSessionsUpdated.RemoveDynamic(this, &UEOSLoginWidget::HandleSessionsUpdated);
        LobbySubsystem->OnJoinSessionComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleJoinLobby);
    }

    LobbySubsystem = nullptr;
    Super::NativeDestruct();
}

void UEOSLoginWidget::RequestEpicLogin()
{
    if (!LobbySubsystem)
    {
        BindToSubsystem();
    }

    if (LobbySubsystem)
    {
        LobbySubsystem->LoginWithEpicAccount();
    }
}

void UEOSLoginWidget::CreateLobby(int32 MaxPublicConnections)
{
    if (!LobbySubsystem)
    {
        BindToSubsystem();
    }

    if (LobbySubsystem)
    {
        LobbySubsystem->CreateLobby(MaxPublicConnections);
    }
}

void UEOSLoginWidget::SearchForLobbies()
{
    if (!LobbySubsystem)
    {
        BindToSubsystem();
    }

    if (LobbySubsystem)
    {
        LobbySubsystem->SearchForLobbies();
    }
}

void UEOSLoginWidget::JoinLobbyByIndex(int32 SessionIndex)
{
    if (!LobbySubsystem)
    {
        BindToSubsystem();
    }

    if (LobbySubsystem)
    {
        LobbySubsystem->JoinLobbyByIndex(SessionIndex);
    }
}

void UEOSLoginWidget::BindToSubsystem()
{
    UEOSLobbySubsystem* PreviousSubsystem = LobbySubsystem;
    LobbySubsystem = nullptr;

    if (PreviousSubsystem)
    {
        PreviousSubsystem->OnLoginComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleLoginComplete);
        PreviousSubsystem->OnCreateSessionComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleCreateLobby);
        PreviousSubsystem->OnSessionsUpdated.RemoveDynamic(this, &UEOSLoginWidget::HandleSessionsUpdated);
        PreviousSubsystem->OnJoinSessionComplete.RemoveDynamic(this, &UEOSLoginWidget::HandleJoinLobby);
    }

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        LobbySubsystem = GameInstance->GetSubsystem<UEOSLobbySubsystem>();
    }

    if (!LobbySubsystem)
    {
        UE_LOG(LogEOSSample, Warning, TEXT("EOSLobbySubsystem not available."));
        return;
    }

    LobbySubsystem->OnLoginComplete.AddDynamic(this, &UEOSLoginWidget::HandleLoginComplete);
    LobbySubsystem->OnCreateSessionComplete.AddDynamic(this, &UEOSLoginWidget::HandleCreateLobby);
    LobbySubsystem->OnSessionsUpdated.AddDynamic(this, &UEOSLoginWidget::HandleSessionsUpdated);
    LobbySubsystem->OnJoinSessionComplete.AddDynamic(this, &UEOSLoginWidget::HandleJoinLobby);

    OnLobbySearchUpdated(LobbySubsystem->GetCachedSessions());
}

void UEOSLoginWidget::HandleLoginComplete(bool bSuccess, const FString& ErrorMessage)
{
    OnLoginStateChanged(bSuccess, ErrorMessage);
}

void UEOSLoginWidget::HandleCreateLobby(bool bSuccess, const FString& ErrorMessage)
{
    OnLobbyCreated(bSuccess, ErrorMessage);
}

void UEOSLoginWidget::HandleSessionsUpdated(const TArray<FSimpleSessionSearchResult>& Results)
{
    OnLobbySearchUpdated(Results);
}

void UEOSLoginWidget::HandleJoinLobby(bool bSuccess, const FString& ConnectionInfo)
{
    OnLobbyJoinComplete(bSuccess, ConnectionInfo);
}
