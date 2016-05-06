# Instruction for Compiling NEXTA/DTALite Source Code

# Introduction #

Requirement:

Visual Studio 2008/2010/2013
SVN version control


Latest version:
DTALite simulation engine: \Version1\_2\DTALite
NEXTA graphical user interface: \Version1\_2\NEXTA-GUI

# Details #

## 1. Download the latest NEXTA/DTALite Release ##
Download a latest release of NEXTA/DTALite at
http://code.google.com/p/nexta/downloads/list

e.g. Internal\_release\_10\_08.zip.

Unzip the zipped file to a local folder: e.g.
C:\NEXTA\_DTALite\Internal\_release.
You will find NEXTA\_64.exe and NEXTA\_32.exe in the installation folder.

There are many gda**_.dll in the installation folder, which are Geospatial Data Abstraction Library (http://www.gdal.org/)_

There are NEXTA project files under subfolder \sample\_data\_sets, e.g. \sample\_data\_sets\6-Node Network.**

You can use NEXTA\_64 to open project file 6node.tnp.

## 2. Install RapidSVN Version Control Package ##

1. Install RapidSVN at http://www.rapidsvn.org/download/

2. Obtain SVN URL at:

https://nexta.googlecode.com/svn/trunk/


3. Create a local folder, say C:\NEXTA\_OpenSource

4. Right-click on bookmarks in RapidSVN, select menu "check-out a new working copy".

5. Use the above SVN URL as the URL input edit, e.g. https://nexta.googlecode.com/svn/trunk/

6. Use the local folder, e.g. C:\NEXTA\_OpenSource, in the Destination Directory.

7. The DTALite source code can be found under  "C:\NEXTA\_OpenSource\Version1\_01\DTALite"

NEXTA GUi source code can be found under  "C:\NEXTA\_OpenSource\Version1\_01\NEXTA-GUI"


## 3. Use Visual C++ 2008/2010/2013 to build the NEXTA package ##

1. Open Visual C++, Click on menu File -> Open ->Project/Solution, Open file  C:\NEXTA\_OpenSource\Version1\_0\NEXTA-GUI\TLite.sln to open NEXTA project.

2. Go to Visual C++ menu Project -> NEXTA Property -> Configuration Property -> Debugging -> Working Directory.

Add the installation folder (e.g. C:\NEXTA\_DTALite\Internal\_release) as the Working Directory. Without this directory, the application will exit when it is launched.

When you use either release or debug mode, either Win 32 or x64 mode, you need to input this working directory for each configuration individually.

3. Create a local folder, c:\NEXTA\_DTALite\_SoftwareRelease , so the compiler/linker can output the following 4 files: DTALite\_32.exe, DTALite\_64.exe and NEXTA\_32.exe and NEXTA\_64.exe to this folder. To change the output folder, please go to Visual C++ menu Project -> NEXTA Property -> Configuration Property ->Linker->General->output files

Compile the source code, and run the NEXTA application.

## 4. Use Visual C++ 2008/2010 to build the DTALite package ##

1. Open Visual C++, Click on menu File -> Open ->Project/Solution, Open file  C:\NEXTA\_OpenSource\Version1\_01\DTALite\DTALite.sln to open DTALite project.

2. Go to Visual C++ menu Project -> DTALite Property -> Configuration Property -> Debugging -> Working Directory.

Add a project folder (e.g. C:\NEXTA\_DTALite\Internal\_release\sample\_data\_sets\Salt\_Lake\_City\_West\_Jordan\) as the Working Directory. Without this directory, the DTALite application will exit when it is launched.

When you use either release or debug mode, either Win 32 or x64 mode, you need to input this working directory for each configuration individually.

3. Compile the source code, and run the DTALite application.