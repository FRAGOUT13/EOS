#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EOSLoginGameInstance.generated.h"

class UEOSLoginWidget;

/**
 * GameInstance that ensures the EOS lobby subsystem is initialized and optionally spawns the login widget.
 */
UCLASS()
class EOSLOGINSAMPLE_API UEOSLoginGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable, Category = "EOS|UI")
    UEOSLoginWidget* GetLoginWidget() const { return LoginWidgetInstance; }

protected:
    /** Widget class to instantiate on startup. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EOS|UI")
    TSubclassOf<UEOSLoginWidget> LoginWidgetClass;

    /** Automatically create and add the login widget to the viewport when the game instance starts. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EOS|UI")
    bool bCreateWidgetOnInit = true;

private:
    UPROPERTY()
    UEOSLoginWidget* LoginWidgetInstance = nullptr;
};
