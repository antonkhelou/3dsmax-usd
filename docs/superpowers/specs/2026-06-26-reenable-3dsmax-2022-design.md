# Re-enable 3ds Max 2022 build support

**Date:** 2026-06-26
**Branch:** `kheloua/reenable-2022` (off `dev`)
**Goal:** Produce a branch where the USD-for-3dsMax plugin **compiles and links** when targeting 3ds Max 2022. Tests and packaging are out of scope (best-effort only).

## Context

Support for 2022 (and 2023) was dropped incrementally as newer Max versions were added. Inspection shows most of the 2022-era build plumbing was *never fully removed* — the `.props` files still carry conditions for `<='2024'`, `<'2024'`, `<'2025'`, so re-enabling is mostly: add 2022 to the version whitelist, fill a few `==2024`-only conditions with a 2022 equivalent, and guard C++ that uses post-2022 Max APIs.

3ds Max 2022 ABI facts: `MAX_VERSION_MAJOR == 24`. Toolset **v141** (required by the installed 2022 Max SDK; satisfiable by MSVC 14.16 present in VS2022). USD **0.21.11**, Python **3.7**, Qt **5.15.1**, Boost **1.70**, PySide2.

## Dependency paths (verified on disk)

| Dep | Path |
|-----|------|
| devkit (`--maxusddevkit`) | `D:\devkit-0.10-2022` |
| maxsdk (`--maxsdk`) | `C:\Program Files\Autodesk\3ds Max 2022 SDK\maxsdk` |
| gtest (`--googletest`) | `D:\artifactory\unzipped\gtest\1.8.1-3dsmax-vc141-001` |
| Qt (`--qtinstall`) | `D:\artifactory\unzipped\Qt\5.15.1-3dsmax-...-vc141` |
| materialx (`--materialx`) | `D:\artifactory\unzipped\2022_3dsmax-component-materialX\...` |
| pybind11 (`--pybind11inc`) | `D:\git\pybind11\include` |
| pyopengl (`--pyopengl`) | Python 3.7 site-packages |

## Build-config changes

1. **`build-scripts/build-solution.py`** — add `2022` to `choices=[...]`.
2. **`src/3dsmax.common.settings.props`** — extend the `PySideLibraries` / `PySideDelayLoadDLL` `=='2024'` PySide2 conditions to also cover 2022 (Python 3.7 PySide2). Verify Qt5 (`<='2024'` → 5.15.1) path.
3. **`src/USD.props`** — add `PythonVersion` for 2022 → `37` (currently only `==2024`→310 and ≥2025). Confirm Boost (`<'2024'`→1_70) and toolset (`<'2025'`→vc141) resolve.
4. **MaxRestrictedSdk/2022** — already present, reuse.
5. **Package script** — legacy menu path (`<2025`) already handles 2022; not needed for compile+link.

## C++ source changes (iterative)

Build, collect compile/link errors, and guard each post-2022 API usage with `#if MAX_VERSION_MAJOR >= NN` (or existing `IS_MAX20XX_OR_GREATER` macros), so 2022 takes the legacy path and newer versions are unaffected. Repeat until the plugin links.

## Definition of done

`build-solution.py ... release 2022` compiles and links the core plugin projects (maxUsd, MaxUsdObjects, preferences, UFEUI). Existing version targets (2024–2027) must remain unaffected (config changes are strictly additive / version-gated).

## Risks

- Toolset/ABI mismatch (v141 vs vc142 artifacts) surfacing at link time → adjust artifact variant.
- Volume of post-2022 API usages unknown → handled by the iterate-on-errors loop.
- USD 21.11 API differences vs newer (some already guarded by `USD_VERSION_*` defines).

## Outcome (2026-06-26)

The full solution (plugin **and** tests) compiles and links for 2022: `Build succeeded. 0 Error(s)`.
All plugin modules are produced: `maxUsd.dll`, `MaxUsdObjects.dlo`, `USDExport.dle`, `USDImport.dli`,
`RenderDelegate.dll`, `UfeUi.dll`, `MaxUsd_Preferences.dll`, plus the USD/boost/ufe runtime DLLs.

### Environment notes (this machine)

`build-solution.py` could not be used as-is because its `configure-vsdevcmd.bat` selects the *latest*
Build Tools, which resolved to a **VS 2026 (v18)** install whose C++ targets are incomplete. The build
must run under **VS 2022 (v17)**. The exact validated invocation (see `build/local-build-2022.bat`):

```
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
MSBuild.exe src\usd-component.sln /m ^
  /p:Configuration=release /p:Platform=x64 /p:VersionTarget=2022 ^
  /p:PlatformToolset=v143 ^                              REM v141 (SDK default) not installed; v143 is binary-compatible
  /p:WindowsTargetPlatformVersion=10.0.19041.0 ^         REM 2022 SDK pins 10.0.17134.0 which is not installed
  /p:MaxSDK="C:\Program Files\Autodesk\3ds Max 2022 SDK\maxsdk" ^
  /p:QtInstall="D:\artifactory\unzipped\Qt\5.15.1-3dsmax-29-vc141\Qt\5.15.1" ^
  /p:PyBind11Inc="D:\git\pybind11-2.13\include" ^        REM pybind11 v2.13.6 worktree (last to support Python 3.7)
  /p:MaterialXDir="D:\artifactory\unzipped\2022_3dsmax-component-materialX\1.0.0-main_550" ^
  /p:GoogleTestDir="D:\artifactory\unzipped\gtest\1.8.1-3dsmax-vc141-001\gtest" ^
  /p:PyOpenGLDir="D:\artifactory\unzipped\PyOpenGL\3.1.5-cp37\PyOpenGL" ^
  /p:MaxUsdDevKit="D:\devkit-0.10-2022"
```

### Issues resolved beyond the original plan

- **fmt 10.x `is_char<wchar_t>` (C2908/C2766):** the 2022 devkit's spdlog bundles fmt 10.x; including
  `<spdlog/fmt/bundled/xchar.h>` early (guarded by `MAX_2022`) in each USD-heavy pch registers the
  specialization before USD/spdlog instantiate it.
- **Newer-than-2022 component APIs guarded with `#ifndef MAX_2022`:** UsdLayerEditor DCC callbacks
  (`setUpdateDCCObjectRootLayerFunction`, `setDCCSceneLocationFunc`, `setDCCWorkspaceSceneLocationFunc`,
  `UIUtils::setErrorDisplayCallbackFunction`), `SaveLayersDialog::saveLayerFilePathUI` (replaced with a Qt
  `getSaveFileName` fallback for 2022), and UsdUfe `UsdUndoDeleteCommand`/`UsdUndoRenameCommand`.
- **Max 2022 SDK differences:** `<triangulate.h>` (vs `Geom/`), `INodeTab::Count()` (vs `size()`),
  `ProgressStart` 4-arg form, `MAXScript::ScriptSource::Dynamic` → `static_cast<…>(3)`, explicit
  `#include <QPushButton>` (Qt5), C4275 suppressed for boost 1.70 python headers in the translators project.
- **UFE lib name:** `ufe_5.lib` for 2022 (vs `ufe_6.lib`).
