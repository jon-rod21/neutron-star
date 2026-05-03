# Neutron Star Render in OpenGL

## How to Run: 

EITHER:
Clone this repo then run the make_build.sh script and you should be good to go. (linux/macos users)
OR for windows users:
Download the project folder and open in the visual studio IDE
	- Ensure you have moved into the project directory in the terminal/powershell before running any commands below
	- The executable will be produced in the build/Debug folder and you will need to run the "neutron" executable after following the below instructions.


To delete/rebuild the build folder (on windows at least), do:
Remove-Item -Recurse -Force build
Then follow up with the instructions below.

For Windows, results may vary so it is best to follow the below instructions:

The project uses openAL soft so first download it from https://openal-soft.org/
Install the openal-soft-1.25.1-bin.zip file and extract it into C:\
Ideally, do not put it anywhere aside from C:\ else you will have to change the pathing in cmake generation.
Rename the extracted folder "openAL" and check to ensure it contains "include", "libs", and "bin" folders.

Next, you will need to run the cmake generation script, just copy and paste the following into your terminal:

cmake `
 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 `
 -DOPENAL_INCLUDE_DIR="C:/OpenAL/include" `
 -DOPENAL_LIBRARY="C:/OpenAL/libs/Win64/OpenAL32.lib" `
 -B build

Then run the build command:

cmake --build build

Finally, you should be able to find the executable "neutron" in the build/Debug folder but do not run it yet.
Next, you will need to copy the openAL dll file from C:/OpenAL/bin/Win64 into the same folder as the executable (build/Debug).
The file itself is usually named "soft_oal.dll" but name doesn't matter as it should be the only file in the Win64 folder.
Once you have copied the dll file into the build/Debug folder, you can run the "neutron" executable.


Dependencies needed are as follows: 

- GLEW, GLAD, GLM, CMake
