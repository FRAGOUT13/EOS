using UnrealBuildTool;
using System.Collections.Generic;

public class EOSLoginSampleEditorTarget : TargetRules
{
    public EOSLoginSampleEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
        ExtraModuleNames.AddRange(new string[] { "EOSLoginSample" });
    }
}
