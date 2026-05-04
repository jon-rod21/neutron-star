Neutron Star Render in OpenGL

==================================================
DOWNLOADING THE PROJECT
==================================================

Option 1 — Clone the Repository (Recommended)

Clone the repository using Git:

git clone <REPOSITORY_URL>
cd neutron-star


--------------------------------------------------

Option 2 — Download ZIP from GitHub

1. Click the green "Code" button on the GitHub repository page.
2. Select "Download ZIP".
3. Extract the ZIP file.
4. Open the extracted "neutron-star" folder.


--------------------------------------------------

Option 3 — Download ZIP from eLearning / Course Website

1. Download the provided ZIP file.
2. Extract the ZIP file.
3. Open the extracted "neutron-star" source folder in:

Visual Studio Community 2026


==================================================
HOW TO RUN
==================================================

LINUX / macOS

Run:

./make_build.sh


==================================================
WINDOWS SETUP
==================================================

1. INSTALL OPENAL SOFT

Download OpenAL Soft from:

https://openal-soft.org/

Download:

openal-soft-1.25.1-bin.zip

Extract it into:

C:\

Rename the extracted folder to:

OpenAL

The folder should contain:

- include
- libs
- bin


--------------------------------------------------

2. GENERATE THE BUILD FILES

Open PowerShell inside the "neutron-star" project folder and run:

cmake ^
 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ^
 -DOPENAL_INCLUDE_DIR="C:/OpenAL/include" ^
 -DOPENAL_LIBRARY="C:/OpenAL/libs/Win64/OpenAL32.lib" ^
 -B build


--------------------------------------------------

3. BUILD THE PROJECT

Run:

cmake --build build

The executable will be generated in:

neutron-star/build/Debug


--------------------------------------------------

4. COPY THE OPENAL DLL

Copy the DLL file from:

C:/OpenAL/bin/Win64

into:

neutron-star/build/Debug

The DLL is usually named:

soft_oal.dll


--------------------------------------------------

5. RUN THE PROGRAM

Run the executable:

neutron

from:

neutron-star/build/Debug


==================================================
REBUILDING THE BUILD FOLDER (WINDOWS)
==================================================

To delete and regenerate the build folder:

Remove-Item -Recurse -Force build

Then repeat the build steps above.


==================================================
DEPENDENCIES
==================================================

- GLEW
- GLAD
- GLM
- CMake
- OpenAL Soft
