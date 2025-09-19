#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSLobbySubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSimpleSessionSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "EOS")
    FString SessionId;

    UPROPERTY(BlueprintReadOnly, Category = "EOS")
    FString OwningUserName;

    UPROPERTY(BlueprintReadOnly, Category = "EOS")
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "EOS")
    int32 OpenSlots = 0;

    UPROPERTY(BlueprintReadOnly, Category = "EOS")
    FString SessionType;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSignedIn, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSSessionCreated, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEOSSessionSearchFinished, const TArray<FSimpleSessionSearchResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEOSJoinLobbyComplete, bool, bSuccess, const FString&, ConnectionInfo);

/**
 * Game instance subsystem that wraps common Epic Online Services login and lobby/session flows.
 */
UCLASS()
class EOSLOGINSAMPLE_API UEOSLobbySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEOSLobbySubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Launches the EOS Account Portal login flow for the local user (index 0). */
    UFUNCTION(BlueprintCallable, Category = "EOS|Auth")
    void LoginWithEpicAccount();

    /** Creates or recreates a lobby session with the desired number of public slots. */
    UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
    void CreateLobby(int32 MaxPublicConnections = 4);

    /** Queries EOS for presence-enabled lobbies. */
    UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
    void SearchForLobbies();

    /** Attempts to join the lobby stored at the provided search index. */
    UFUNCTION(BlueprintCallable, Category = "EOS|Lobby")
    void JoinLobbyByIndex(int32 SessionIndex);

    /** Returns the cached results from the most recent lobby search. */
    UFUNCTION(BlueprintPure, Category = "EOS|Lobby")
    const TArray<FSimpleSessionSearchResult>& GetCachedSessions() const { return CachedSearchResults; }

    UPROPERTY(BlueprintAssignable, Category = "EOS|Auth")
    FOnEOSSignedIn OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "EOS|Lobby")
    FOnEOSSessionCreated OnCreateSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "EOS|Lobby")
    FOnEOSSessionSearchFinished OnSessionsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "EOS|Lobby")
    FOnEOSJoinLobbyComplete OnJoinSessionComplete;

private:
    void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error); 
    void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void HandleFindSessionsComplete(bool bWasSuccessful);
    void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    bool EnsureInterfaces();
    bool IsUserLoggedIn() const;
    void CleanupSessionDelegates();
    void StartCreateSession();

    IOnlineSubsystem* OnlineSubsystem = nullptr;
    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr IdentityInterface;
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    TArray<FSimpleSessionSearchResult> CachedSearchResults;

    FDelegateHandle LoginCompleteHandle;
    FDelegateHandle CreateSessionCompleteHandle;
    FDelegateHandle DestroySessionCompleteHandle;
    FDelegateHandle FindSessionsCompleteHandle;
    FDelegateHandle JoinSessionCompleteHandle;

    const FName LobbySessionName;
    int32 PendingPublicConnections;
    bool bPendingCreateAfterDestroy;
};
