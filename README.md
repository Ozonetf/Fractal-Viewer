# Simple Fractal-Viewer
![Screenshot (4)](https://github.com/Ozonetf/Fractal-Viewer/assets/45222864/71c900f7-6864-4198-b883-c20f194f5a56)
A simple fractal viewer in C++ using DirectX for rendering, build with MS Visaul Studio, allows user to zoom in specific region on the Mandlebroat set and renders it, supports multi-threading for computing.
<h2>Controls</h2>
P: reset render <br>
M: toggle multithreading for computing <br>
Arrow Up/Down: increase/decrease antialiasing factor <br>
Arrow Left/Right: increase/decrease bailout factor  <br>
Mouse: Drag and select region to zoom in <br>
<h2>Bulding Solution:  </h2>
Build with MS Visual Studio 17++, if unsuccessful, make sure to: <br>
Target DXToolKit in project dependencies: right click on project->properties->project dependencies <br> <br>
Additional include directories: Fractal Viewer properties->C/C++->General->Additional Include Directories: $(SolutionDir)\DirectXTK\inc <br> <br>
Linker dependencies: Fractal Viewer properties->Linker->Input->Additional Dependencies: d2d1.lib;d3d11.lib;dxgi.lib;dxguid.lib;uuid.lib;kernel32.lib;user32.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;%(AdditionalDependencies)
