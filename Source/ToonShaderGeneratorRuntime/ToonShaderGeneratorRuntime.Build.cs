
using UnrealBuildTool;

public class ToonShaderGeneratorRuntime : ModuleRules
{
    public ToonShaderGeneratorRuntime(
        ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}
