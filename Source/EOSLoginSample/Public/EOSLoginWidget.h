#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EOSLobbySubsystem.h"
#include "EOSLoginWidget.generated.h"

/**
 * Base widget that exposes high-level EOS actions to Blueprints.
 */
UCLASS(Abstract, BlueprintType)
class EOSLOGINSAMPLE_API UEOSLoginWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "EOS|UI")
    void RequestEpicLogin();

    UFUNCTION(BlueprintCallable, Category = "EOS|UI")
    void CreateLobby(int32 MaxPublicConnections = 4);

    UFUNCTION(BlueprintCallable, Category = "EOS|UI")
    void SearchForLobbies();

    UFUNCTION(BlueprintCallable, Category = "EOS|UI")
    void JoinLobbyByIndex(int32 SessionIndex);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "EOS|UI")
    void OnLoginStateChanged(bool bIsLoggedIn, const FString& ErrorMessage);

    UFUNCTION(BlueprintImplementableEvent, Category = "EOS|UI")
    void OnLobbyCreated(bool bSuccess, const FString& ErrorMessage);

    UFUNCTION(BlueprintImplementableEvent, Category = "EOS|UI")
    void OnLobbySearchUpdated(const TArray<FSimpleSessionSearchResult>& Results);

    UFUNCTION(BlueprintImplementableEvent, Category = "EOS|UI")
    void OnLobbyJoinComplete(bool bSuccess, const FString& ConnectionInfo);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS|UI")
    UEOSLobbySubsystem* GetLobbySubsystem() const { return LobbySubsystem; }

private:
    void BindToSubsystem();

    UFUNCTION()
    void HandleLoginComplete(bool bSuccess, const FString& ErrorMessage);

    UFUNCTION()
    void HandleCreateLobby(bool bSuccess, const FString& ErrorMessage);

    UFUNCTION()
    void HandleSessionsUpdated(const TArray<FSimpleSessionSearchResult>& Results);

    UFUNCTION()
    void HandleJoinLobby(bool bSuccess, const FString& ConnectionInfo);

    UPROPERTY()
    UEOSLobbySubsystem* LobbySubsystem = nullptr;
};
