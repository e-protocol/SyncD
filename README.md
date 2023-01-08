********************************************************************************
## SyncD<br/>
<p float="left">
<img src = "Project Photo/img.jpg" width = "640" height = "360" />
</p>
Created by E-Protocol
https://github.com/e-protocol
<br/>
Platform: Windows 10<br/>
IDE: Visual Studio 2019<br/>
Description:<br/>
This Console app is for fast synchronization files between directories, including disks.<br/>
It uses 100% STL C++17 and compiled with CMake.<br/>
Asynchronous parallel cycle iteration is applied to speed up synchronization process.<br/>
It creates async threads according to number of CPU cores.<br/>
Test info: AMD Ryzen 5 2600 6 cores 3.4 GHz, 2000 files copy, 3000 files delete operations<br/>
~52 seconds without async iteration counterpart to<br/>
~9 seconds with async iteration.<br/>
For fast call in console add location folder with syncD.exe to PATH variable.<br/>
<br/>
********************************************************************************
<br/>