# Building the USD plugin for 3ds Max 2022 (local)

`build-scripts/build-solution.py` selects the newest installed Build Tools, which
can resolve to VS 2026 (incomplete C++ targets). Build the 2022 target under
**VS 2022 (v17)** by driving MSBuild directly. Adjust the dependency paths to your
machine.

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
MSBuild.exe src\usd-component.sln /m ^
  /p:Configuration=release /p:Platform=x64 /p:VersionTarget=2022 ^
  /p:PlatformToolset=v143 ^                        REM v141 (2022 SDK default) not installed; v143 is binary-compatible
  /p:WindowsTargetPlatformVersion=10.0.19041.0 ^   REM 2022 SDK pins 10.0.17134.0, which is not installed
  /p:MaxSDK="C:\Program Files\Autodesk\3ds Max 2022 SDK\maxsdk" ^
  /p:QtInstall="D:\artifactory\unzipped\Qt\5.15.1-3dsmax-29-vc141\Qt\5.15.1" ^
  /p:PyBind11Inc="D:\git\pybind11-2.13\include" ^  REM pybind11 2.13 (last to support Python 3.7)
  /p:MaterialXDir="D:\artifactory\unzipped\2022_3dsmax-component-materialX\1.0.0-main_550\2022_3dsmax-component-materialX" ^
  /p:GoogleTestDir="D:\artifactory\unzipped\gtest\1.8.1-3dsmax-vc141-001\gtest" ^
  /p:PyOpenGLDir="D:\artifactory\unzipped\PyOpenGL\3.1.5-cp37\PyOpenGL" ^
  /p:SpdlogInc="D:\artifactory\unzipped\spdlog\1.6.2\spdlog" ^   REM fmt-6-era spdlog; the 0.10-2022 devkit's spdlog is fmt-10
  /p:MaxUsdDevKit="D:\devkit-0.10-2022"
```
