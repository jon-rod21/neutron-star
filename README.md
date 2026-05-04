# Neutron Star Render in OpenGL

## Downloading the Project

### Option 1 — Clone the Repository (Recommended)

Clone the repository using Git:

```bash
git clone <REPOSITORY_URL>
cd neutron-star
```

---

### Option 2 — Download ZIP from GitHub

- Click the green **Code** button on the GitHub repository page.
- Select **Download ZIP**.
- Extract the ZIP file.
- Open the extracted `neutron-star` folder.

---

### Option 3 — Download ZIP from eLearning / Course Website

- Download the provided ZIP file.
- Extract the ZIP file.
- Open the extracted `neutron-star` source folder in:

```text
Visual Studio Community 2026
```

---

# How to Run

## Linux / macOS

Run:

```bash
./make_build.sh
```

---

## Windows Setup

### 1. Install OpenAL Soft

Download OpenAL Soft from:

https://openal-soft.org/

Download:

```text
openal-soft-1.25.1-bin.zip
```

Extract it into:

```text
C:\
```

Rename the extracted folder to:

```text
OpenAL
```

Your folder should contain:

- `include`
- `libs`
- `bin`

---

### 2. Generate the Build Files

Open PowerShell inside the `neutron-star` project folder and run:

```powershell
cmake `
 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 `
 -DOPENAL_INCLUDE_DIR="C:/OpenAL/include" `
 -DOPENAL_LIBRARY="C:/OpenAL/libs/Win64/OpenAL32.lib" `
 -B build
```

---

### 3. Build the Project

Run:

```powershell
cmake --build build
```

The executable will be generated in:

```text
neutron-star/build/Debug
```

---

### 4. Copy the OpenAL DLL

Copy the DLL file from:

```text
C:/OpenAL/bin/Win64
```

into:

```text
neutron-star/build/Debug
```

The DLL is usually named:

```text
soft_oal.dll
```

---

### 5. Run the Program

Run the executable:

```text
neutron
```

from:

```text
neutron-star/build/Debug
```

---

## Rebuilding the Build Folder (Windows)

To delete and regenerate the build folder:

```powershell
Remove-Item -Recurse -Force build
```

Then repeat the build steps above.

---

# Dependencies

The project uses:

- GLEW
- GLAD
- GLM
- CMake
- OpenAL Soft
