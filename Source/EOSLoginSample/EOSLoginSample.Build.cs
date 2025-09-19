using UnrealBuildTool;

public class EOSLoginSample : ModuleRules
{
    public EOSLoginSample(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "OnlineSubsystemEOS"
        });
    }
}
