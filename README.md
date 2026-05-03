# Neutron Star Render in OpenGL

## How to Run

### Linux / macOS

Clone this repo, then run the `make_build.sh` script and you should be good to go.
To delete/rebuild the build folder (on windows at least), do:
Remove-Item -Recurse -Force build
Then follow up with make_build.sh script again.

Dependencies needed are as follows: 

---

### Windows

Download the project folder and open it in the Visual Studio IDE.

- Ensure you have moved into the project directory in the terminal/PowerShell before running any commands below.
- The executable will be produced in the `neutron-star/build/Debug` folder.
- After following the setup instructions below, run the `neutron` executable.

---

## Rebuilding the Build Folder (Windows)

To delete and rebuild the `build` folder:

```powershell
Remove-Item -Recurse -Force build
```

Then follow the instructions below.

---

## Windows OpenAL Setup

For Windows, results may vary, so it is best to follow the setup below carefully.

The project uses OpenAL Soft.

First, download OpenAL Soft from:

https://openal-soft.org/

Download the:

```text
openal-soft-1.25.1-bin.zip
```

Extract it into:

```text
C:\
```

Ideally, do not place it anywhere else, otherwise you will need to manually change the OpenAL paths during CMake generation.

Rename the extracted folder to:

```text
OpenAL
```

Ensure the folder contains:

- `include`
- `libs`
- `bin`

---

## Generate the CMake Build Files

Run the following command in PowerShell:

```powershell
cmake `
 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 `
 -DOPENAL_INCLUDE_DIR="C:/OpenAL/include" `
 -DOPENAL_LIBRARY="C:/OpenAL/libs/Win64/OpenAL32.lib" `
 -B build
```

---

## Build the Project

Run:

```powershell
cmake --build build
```

---

## OpenAL DLL Setup

After building, you should find the executable:

```text
neutron
```

inside:

```text
neutron-star/build/Debug
```

Do **not** run it yet.

Next, copy the OpenAL DLL file from:

```text
C:/OpenAL/bin/Win64
```

into:

```text
neutron-star/build/Debug
```

The DLL file is usually named:

```text
soft_oal.dll
```

The filename itself does not matter much since it should be the only DLL inside the `Win64` folder.

Once copied into `neutron-star/build/Debug`, you can run the:

```text
neutron
```

executable.

---

## Dependencies

The project uses the following dependencies:

- GLEW
- GLAD
- GLM
- CMake
- OpenAL Soft