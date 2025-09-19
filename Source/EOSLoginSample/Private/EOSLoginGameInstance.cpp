#include "EOSLoginGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "EOSLobbySubsystem.h"
#include "EOSLoginSample.h"
#include "EOSLoginWidget.h"
#include "Engine/World.h"

void UEOSLoginGameInstance::Init()
{
    Super::Init();

    // Force the subsystem to initialize so delegates are ready for widgets.
    GetSubsystem<UEOSLobbySubsystem>();

    if (bCreateWidgetOnInit && LoginWidgetClass)
    {
        if (UWorld* World = GetWorld())
        {
            LoginWidgetInstance = CreateWidget<UEOSLoginWidget>(World, LoginWidgetClass);
            if (LoginWidgetInstance)
            {
                LoginWidgetInstance->AddToViewport();
            }
        }
    }
}

void UEOSLoginGameInstance::Shutdown()
{
    if (LoginWidgetInstance)
    {
        LoginWidgetInstance->RemoveFromParent();
        LoginWidgetInstance = nullptr;
    }

    Super::Shutdown();
}
