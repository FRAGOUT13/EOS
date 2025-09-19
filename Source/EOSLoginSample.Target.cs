using UnrealBuildTool;
using System.Collections.Generic;

public class EOSLoginSampleTarget : TargetRules
{
    public EOSLoginSampleTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
        ExtraModuleNames.AddRange(new string[] { "EOSLoginSample" });
    }
}
