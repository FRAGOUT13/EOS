#include "EOSLobbySubsystem.h"

#include "EOSLoginSample.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h"

UEOSLobbySubsystem::UEOSLobbySubsystem()
    : LobbySessionName(NAME_GameSession)
    , PendingPublicConnections(4)
    , bPendingCreateAfterDestroy(false)
{
}

void UEOSLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    EnsureInterfaces();
}

void UEOSLobbySubsystem::Deinitialize()
{
    CleanupSessionDelegates();
    SessionSearch.Reset();
    SessionInterface.Reset();
    IdentityInterface.Reset();
    OnlineSubsystem = nullptr;

    Super::Deinitialize();
}

void UEOSLobbySubsystem::LoginWithEpicAccount()
{
    if (!EnsureInterfaces())
    {
        OnLoginComplete.Broadcast(false, TEXT("Online subsystem unavailable."));
        return;
    }

    if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        OnLoginComplete.Broadcast(true, TEXT("Already logged in."));
        return;
    }

    if (LoginCompleteHandle.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteHandle);
        LoginCompleteHandle.Reset();
    }

    LoginCompleteHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::HandleLoginComplete));

    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("accountportal");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    if (!IdentityInterface->Login(0, Credentials))
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteHandle);
        LoginCompleteHandle.Reset();
        OnLoginComplete.Broadcast(false, TEXT("Failed to start EOS login."));
    }
}

void UEOSLobbySubsystem::CreateLobby(int32 MaxPublicConnections)
{
    PendingPublicConnections = FMath::Max(1, MaxPublicConnections);

    if (!EnsureInterfaces())
    {
        OnCreateSessionComplete.Broadcast(false, TEXT("Online subsystem unavailable."));
        return;
    }

    if (!IsUserLoggedIn())
    {
        OnCreateSessionComplete.Broadcast(false, TEXT("User is not logged in."));
        return;
    }

    if (SessionInterface->GetNamedSession(LobbySessionName) != nullptr)
    {
        bPendingCreateAfterDestroy = true;
        if (DestroySessionCompleteHandle.IsValid())
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
            DestroySessionCompleteHandle.Reset();
        }

        DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::HandleDestroySessionComplete));
        if (!SessionInterface->DestroySession(LobbySessionName))
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
            DestroySessionCompleteHandle.Reset();
            bPendingCreateAfterDestroy = false;
            OnCreateSessionComplete.Broadcast(false, TEXT("Failed to destroy existing session."));
        }
        return;
    }

    StartCreateSession();
}

void UEOSLobbySubsystem::StartCreateSession()
{
    if (!EnsureInterfaces())
    {
        OnCreateSessionComplete.Broadcast(false, TEXT("Online subsystem unavailable."));
        return;
    }

    const FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnCreateSessionComplete.Broadcast(false, TEXT("Invalid local user id."));
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = PendingPublicConnections;
    SessionSettings.NumPrivateConnections = 0;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bUseLobbiesIfAvailable = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.Set(TEXT("SESSION_NAME"), FString(TEXT("EOSLobby")), EOnlineDataAdvertisementType::ViaOnlineService);

    if (CreateSessionCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
        CreateSessionCompleteHandle.Reset();
    }

    CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::HandleCreateSessionComplete));

    if (!SessionInterface->CreateSession(*UserId, LobbySessionName, SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
        CreateSessionCompleteHandle.Reset();
        OnCreateSessionComplete.Broadcast(false, TEXT("CreateSession request failed to start."));
    }
}

void UEOSLobbySubsystem::SearchForLobbies()
{
    if (!EnsureInterfaces())
    {
        OnSessionsUpdated.Broadcast({});
        return;
    }

    if (FindSessionsCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
        FindSessionsCompleteHandle.Reset();
    }

    SessionSearch = MakeShared<FOnlineSessionSearch>();
    SessionSearch->MaxSearchResults = 50;
    SessionSearch->bIsLanQuery = false;
    SessionSearch->PingBucketSize = 50;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    FindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::HandleFindSessionsComplete));

    const FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
        FindSessionsCompleteHandle.Reset();
        OnSessionsUpdated.Broadcast({});
        return;
    }

    if (!SessionInterface->FindSessions(*UserId, SessionSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
        FindSessionsCompleteHandle.Reset();
        OnSessionsUpdated.Broadcast({});
    }
}

void UEOSLobbySubsystem::JoinLobbyByIndex(int32 SessionIndex)
{
    if (!EnsureInterfaces() || !SessionSearch.IsValid())
    {
        OnJoinSessionComplete.Broadcast(false, TEXT("Session search invalid."));
        return;
    }

    if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex))
    {
        OnJoinSessionComplete.Broadcast(false, TEXT("Invalid session index."));
        return;
    }

    const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SessionIndex];

    if (JoinSessionCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
        JoinSessionCompleteHandle.Reset();
    }

    JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::HandleJoinSessionComplete));

    const FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
        JoinSessionCompleteHandle.Reset();
        OnJoinSessionComplete.Broadcast(false, TEXT("Invalid local user id."));
        return;
    }

    if (!SessionInterface->JoinSession(*UserId, LobbySessionName, Result))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
        JoinSessionCompleteHandle.Reset();
        OnJoinSessionComplete.Broadcast(false, TEXT("JoinSession request failed to start."));
    }
}

void UEOSLobbySubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (IdentityInterface.IsValid() && LoginCompleteHandle.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginCompleteHandle);
        LoginCompleteHandle.Reset();
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogEOSSample, Log, TEXT("EOS login successful: %s"), *UserId.ToString());
        OnLoginComplete.Broadcast(true, FString());
    }
    else
    {
        UE_LOG(LogEOSSample, Warning, TEXT("EOS login failed: %s"), *Error);
        OnLoginComplete.Broadcast(false, Error);
    }
}

void UEOSLobbySubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid() && CreateSessionCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
        CreateSessionCompleteHandle.Reset();
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogEOSSample, Log, TEXT("Session created successfully: %s"), *SessionName.ToString());
        OnCreateSessionComplete.Broadcast(true, FString());
    }
    else
    {
        UE_LOG(LogEOSSample, Warning, TEXT("Session creation failed: %s"), *SessionName.ToString());
        OnCreateSessionComplete.Broadcast(false, TEXT("Session creation failed."));
    }
}

void UEOSLobbySubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid() && DestroySessionCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
        DestroySessionCompleteHandle.Reset();
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogEOSSample, Warning, TEXT("Failed to destroy session: %s"), *SessionName.ToString());
        OnCreateSessionComplete.Broadcast(false, TEXT("Failed to destroy existing session."));
        bPendingCreateAfterDestroy = false;
        return;
    }

    UE_LOG(LogEOSSample, Log, TEXT("Destroyed existing session: %s"), *SessionName.ToString());

    if (bPendingCreateAfterDestroy)
    {
        bPendingCreateAfterDestroy = false;
        StartCreateSession();
    }
}

void UEOSLobbySubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
    if (SessionInterface.IsValid() && FindSessionsCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
        FindSessionsCompleteHandle.Reset();
    }

    CachedSearchResults.Empty();

    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        OnSessionsUpdated.Broadcast(CachedSearchResults);
        return;
    }

    for (int32 Index = 0; Index < SessionSearch->SearchResults.Num(); ++Index)
    {
        const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[Index];
        FSimpleSessionSearchResult BlueprintResult;
        BlueprintResult.SessionId = Result.GetSessionIdStr();
        BlueprintResult.OwningUserName = Result.Session.OwningUserName;
        BlueprintResult.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
        BlueprintResult.OpenSlots = Result.Session.NumOpenPublicConnections;

        FString SessionType;
        Result.Session.SessionSettings.Get(TEXT("SESSION_NAME"), SessionType);
        BlueprintResult.SessionType = SessionType;

        CachedSearchResults.Add(BlueprintResult);
    }

    OnSessionsUpdated.Broadcast(CachedSearchResults);
}

void UEOSLobbySubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface.IsValid() && JoinSessionCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
        JoinSessionCompleteHandle.Reset();
    }

    FString ConnectString;
    bool bSuccess = false;

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            bSuccess = true;
        }
    }

    if (bSuccess)
    {
        UE_LOG(LogEOSSample, Log, TEXT("Join session succeeded: %s"), *SessionName.ToString());
        OnJoinSessionComplete.Broadcast(true, ConnectString);
    }
    else
    {
        UE_LOG(LogEOSSample, Warning, TEXT("Join session failed: %s"), *UEnum::GetValueAsString(Result));
        OnJoinSessionComplete.Broadcast(false, ConnectString);
    }
}

bool UEOSLobbySubsystem::EnsureInterfaces()
{
    if (!OnlineSubsystem)
    {
        OnlineSubsystem = IOnlineSubsystem::Get();
        if (!OnlineSubsystem)
        {
            UE_LOG(LogEOSSample, Error, TEXT("Online subsystem unavailable."));
            return false;
        }
    }

    if (!SessionInterface.IsValid())
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (!SessionInterface.IsValid())
        {
            UE_LOG(LogEOSSample, Error, TEXT("Session interface unavailable."));
            return false;
        }
    }

    if (!IdentityInterface.IsValid())
    {
        IdentityInterface = OnlineSubsystem->GetIdentityInterface();
        if (!IdentityInterface.IsValid())
        {
            UE_LOG(LogEOSSample, Error, TEXT("Identity interface unavailable."));
            return false;
        }
    }

    return true;
}

bool UEOSLobbySubsystem::IsUserLoggedIn() const
{
    return IdentityInterface.IsValid() && IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

void UEOSLobbySubsystem::CleanupSessionDelegates()
{
    if (IdentityInterface.IsValid() && LoginCompleteHandle.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteHandle);
        LoginCompleteHandle.Reset();
    }

    if (SessionInterface.IsValid())
    {
        if (CreateSessionCompleteHandle.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
            CreateSessionCompleteHandle.Reset();
        }

        if (DestroySessionCompleteHandle.IsValid())
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
            DestroySessionCompleteHandle.Reset();
        }

        if (FindSessionsCompleteHandle.IsValid())
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
            FindSessionsCompleteHandle.Reset();
        }

        if (JoinSessionCompleteHandle.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
            JoinSessionCompleteHandle.Reset();
        }
    }
}
