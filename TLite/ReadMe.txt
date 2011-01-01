To Do List.

// to do: delete m_TrainVector; m_node, link set;
memory clean up

. load file status, just like TRANSIMS loading utility

1. add link directly
2. show distance grid
3. save functions
4. show node numbers in the center of node
5. show link arrow


1. read train information,+
2. list train info +
3. display train timetable -

5. time-dependent shortest path to calculate timetable given resource price vector
6. calculate resource price vector for additional constraints
7. performance display
8. 3D visutalization


1. In 3D view, change height as volume, color as reliability measurement
2. Add node, link and vehicle list


================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : TLite Project Overview
===============================================================================

The application wizard has created this TLite application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your TLite application.

TLite.vcproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

TLite.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CTLiteApp application class.

TLite.cpp
    This is the main application source file that contains the application
    class CTLiteApp.

TLite.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\TLite.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file TLite.rc.

res\TLite.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CMDIFrameWnd and controls all MDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in TLite.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

For the child frame window:

ChildFrm.h, ChildFrm.cpp
    These files define and implement the CChildFrame class, which
    supports the child windows in an MDI application.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

TLiteDoc.h, TLiteDoc.cpp - the document
    These files contain your CTLiteDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CTLiteDoc::Serialize).

TLiteView.h, TLiteView.cpp - the view of the document
    These files contain your CTLiteView class.
    CTLiteView objects are used to view CTLiteDoc objects.

res\TLiteDoc.ico
    This is an icon file, which is used as the icon for MDI child windows
    for the CTLiteDoc class.  This icon is included by the main
    resource file TLite.rc.




/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named TLite.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC90XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////
