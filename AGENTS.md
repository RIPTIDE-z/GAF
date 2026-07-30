# GAF Agent Instructions

## Project Overview

- GAF is an Unreal Engine 5.8 plugin under `Learn_Test_Project/Plugins/GAF`
- The plugin descriptor file is `GameAnimationFramework.uplugin`
- The main content of the plugin is a modular, reusable UE character animation system, intended to integrate animation frameworks such as Als, Lyra, and GASP
  - The Als and Lyra framework reference code is located in `GAF\Reference\RefCode\AlsRefactoredSource` and `GAF\Reference\RefCode\LyraSource`
  - Unless the user explicitly requests it, do not read this reference code
- Unless the user explicitly requests it, do not read or modify content in other plugins outside `GAF`
- If asked to write documentation files, always place them in `GAF\Reference\Document`

## Modification Scope And Coding Rules

- C++ code follows Unreal style and the project-root `GAF/.clang-format`
- `GAF/.clang-format` is a UE-oriented configuration: 4-wide indentation, Allman braces, `Type* Ptr` / `Type& Ref` pointer and reference style, and include sorting is disabled to protect Unreal generated include order
- Do not sort or move includes related to Unreal generated code. In particular, keep `.generated.h` as the last include in the corresponding header file, and keep `UE_INLINE_GENERATED_CPP_BY_NAME(...)` after the include section in source files
- When modifying module names, export macros, reflected type names, or `.uplugin` module declarations, you must also check Build.cs, includes, generated includes, `IMPLEMENT_MODULE`, and module dependencies
- All important code must include clear but concise English comments
  - do not use a period at the end of a sentence

## Required Checks

- After every GAF code change, you must execute the following in order:

```bat
clang-format --style=file:GAF/.clang-format -i <changed C++ files>
cmd /c .\GAF\BuildPlugin.bat
```

- If you are not sure which C++ files were changed, you can run `clang-format` on all `.h`, `.hpp`, `.cpp`, and `.cxx` files under `GAF/Source`. After formatting, it is recommended to run one more dry-run:

```bat
clang-format --style=file:GAF/.clang-format --dry-run -Werror <formatted C++ files>
```

- `GAF/BuildPlugin.bat` is this plugin's unified build validation script. The script calls Unreal AutomationTool's `BuildPlugin`, and the output directory remains `GAF/Build`
  If the build fails, fix the cause of the failure first, then rerun the same script

## Required Output

- Every time you modify the code, you must specify which parts were changed and the reason for the changes
