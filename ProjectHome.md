# NeXTA: Network EXplorer for Traffic Analysis #
# DTALite: Light-weight Dynamic Traffic Assignment Engine #
Email List: dtalite-nexta-email-list@googlegroups.com

## Data Structure and Workflow ##

[Data Structure and workflow of DTALite and NeXTA](https://docs.google.com/document/d/1z4YsztPXcWfQAd8NVD4_KXv7hJcbyXdYPzwL6z8xn3U)

[Open Data Hub Schema using NeXTA](https://docs.google.com/document/d/1d1Zhnhm-QnCdOpqoe4-EO0U8I4ej17JprGbSgo0zNxU/edit?usp=drive_web)

[Introduction to DTALite and NeXTA using a work zone application](https://docs.google.com/document/d/1OyeqDPvjev0eZYE-EMTZI2NMoOhqASQi_40gz3ByvIg/edit)

[DTALite: A queue-based mesoscopic traffic simulator for fast model evaluation and calibration. Xuesong Zhou, Jeffrey Taylor. Cogent Engineering, Vol. 1, Iss. 1, 2014](http://www.tandfonline.com/doi/full/10.1080/23311916.2014.961345#.VDILIfldXo8)

## Users' Guide ##

[NeXTA Questions and Answers ](https://docs.google.com/document/d/1zleCAeq6CfdPBmk9DkHSmlob82qnMaShrshgvZtfTbQ)

[How-to Guide: Use NeXTA Visualization Features](https://docs.google.com/presentation/d/1LIBbko7nSWvcpkIOgB1jprJrMiIealD2nxhV3SgzIKY/edit?usp=sharing)



[Related Learning Documents on important data, perform traffic assignment and simulation](http://www.learning-transportation.org)

## Software Release ##
Announcement:

Since the NeXTA and DTALite have not been updated for about one year, please click the following link for the instruction of how to download and use the latest version.
https://docs.google.com/document/d/1IBQNZewQZcuk0gF7QxQTXiaXnKlnidkJaFKQj2OrAyM/edit


[Latest Software Release](http://code.google.com/p/nexta/downloads/)

Contact persons: Dr. Xuesong Zhou at Arizona State University (xzhou74@asu.edu); Jeffrey Taylor (jeffrey.taylor.d@gmail.com)

If you encounter a message showing "mfc120.dll is missing from computer:, please download Visual C++ redistribution package for 2013
http://www.microsoft.com/en-us/download/details.aspx?id=40784
as this package is required for our OpenMP-based parallel computing.

## Webinar Videos ##
[Youtube site](http://www.youtube.com/channel/UCUHlqojCQ4f7VvqroUhbaFA)|
[Introduction & Project Overview](http://www.youtube.com/watch?v=QHMshz6PauY)|
[Capabilities & Modeling Approach](http://www.youtube.com/watch?v=VxNz0Oi5qoo)|
[Software Demonstration](http://www.youtube.com/watch?v=lxFFDvwPtSY)|
[Introduction to Homework Assignment](http://www.youtube.com/watch?v=taIlhCl2Wic)

## Description ##

Network EXplorer for Traffic Analysis (NEXTA) (Version 3) is an open-source GUI that aims to facilitate the preparation, post-processing and analysis of transportation assignment, simulation and scheduling datasets.  NeXTA Version 3 uses [DTALite](http://sourceforge.net/projects/dtalite/), a fast dynamic traffic assignment engine, for transportation network analysis.

DTALite uses a computationally simple but theoretically rigorous traffic queuing model in its lightweight mesoscopic simulation engine. Its built-in parallel computing capability dramatically speeds-up the analysis process by using widely available multi-core CPU hardware. It takes about 1 hour to compute agent-based dynamic traffic equilibrium for a large-scale network with 1 million vehicles for 20 iterations.


NEXTA Version 2 has been widely used as the front-end GUI for DYNASMART and TRANSIMS. [NEXTA Version 2 for TRANSIMS](http://www.transims-opensource.net/)


## DOT and MPO Reports on Application of DTALite/NEXTA ##
**FHWA Project: The Effective Integration of Analysis, Modeling, and Simulation Tools**
Test Networks from Portland Metro and Pima Association of Governments
http://www.fhwa.dot.gov/publications/research/operations/13036/index.cfm

**North Carolina DOT Project: Work Zone Traffic Analysis & Impact Assessment**
http://www.ncdot.gov/doh/preconstruct/tpb/research/download/2012-36finalreport.pdf


**Strategic Highway Research Program (SHRP 2) Transportation Research Board C05 Understanding the Contributions of Operations, Technology, and Design to
Meeting Highway Capacity Needs**
http://sites.kittelson.com/SHRP2_C05/Downloads/Download/956

## Motivation ##

NeXTA Version 3 aims to:

  * provide a clean open-source code base to enable transportation researchers and software developers to continue to build upon and expand its range of capabilities to other simulation/optimization packages.

  * make critical issues visible to researchers by visualizing time-varying traffic flow dynamics and traveler route choice behavior in an integrated 2D/3D environment.

  * provide a free education tool for students to understand the complex decision-making process in transportation planning and optimization processes. In this version, "e" in "NeXTA" is for Education.


## Educational Tools ##

Initial efforts have begun to develop a step-by-step tutorial and a self-study software package for students to learn advanced modeling skills in transportation planning applications. To teach the essential 4-step process (trip generation, trip distribution, modal split and traffic assignment) in transportation planning and introduce students to using NeXTA, a step-by-step tutorial will introduce students to building simplified transportation networks, and accompanying spreadsheets illustrate the complex computational details of the 4-step process. Using NeXTA’s traffic simulation tools, the self-study guide will assist students in understanding important transportation planning principles in high-impact planning applications, such as how to assess impacts of earthquakes on metropolitan regions and how to systematically evaluate bridge retrofit scenarios.


## Features ##

NeXTA Version 3 now has the following capabilities:

### 1. Create, import, edit, store, export and visualize transportation network data. ###

![http://imageshack.us/a/img214/3022/nextagui600.png](http://imageshack.us/a/img214/3022/nextagui600.png)

_Chicago network (1,790 zones; 12,982 nodes; 39,018 links) converted from static assignment network data sets at http://www.bgu.ac.il/~bargera/tntp/_

![http://imageshack.us/a/img695/9897/3d2dwithbackgroundimage.png](http://imageshack.us/a/img695/9897/3d2dwithbackgroundimage.png)

The NEXTA now support importing the following data
1.	GIS shape files
2.	Synchro UTDF and combined CSV files
3.	TMC-based sensor data (e.g. Inrix or traffic.com)
4.	Import google transit feed data
5.	Open dynsmart data set for reliability analysis
6.	NGSim data for vehicle analysis
7.	RAS train timetable format


NEXTA also can export to the following format.
1.	Google Earth KML
2.	GIS shape file: node/link/zone, or convert user-defined CSV to shape
3.	synchro (UTDF)
4.	Use QEM tool to determine timing data
5.	Export to Visim ANM format
6.	Identify signal location, estimate signal timing based on QEM tool
7.	Subarea analysis to provide path flow pattern from Vissim simulation


> _The height (Z-axis) represents link-specific lane-volume, the color range of those vertical strips between red and green represents travel time variability (quantified as travel speed standard deviation at peak hours). Red: large variability, green: low variability. From this graph, it is easy to see that large volume does not necessarily lead to high travel time variability._


The DTALite package provides the following unique features using the AMS data hub format through NEXTA.

1.	Unlimited number of link types
2.	Unlimited number of demand types/demand files, 24 hour loading period: Flexible network conversion and linkage with GIS Shapefile (importing, script for mapping planning data to our data hub): save time, allow flexible number of link types and node types)
3.	Flexible demand data format: 3-column (o,d, value), multiple columns (o,d, SOV, HOV, Truck, subtotal), matrix, with 15-min departure time interval. Agent file
4.	Typical vehicle types: mapping from trip types to vehicle types, vehicle emission rates for different vehicle types, different ages
5.	Semi-continuous Value Of Time distribution
6.	Common types of sensor data, link count, lane count, at user-defined interval, speed data, density data, route travel time data,
7.	Unlimited number of safety prediction models, based on link volume, length, link type, # of intersections/drive ways per miles
8.	Movement-specific parameters (based on HCM/QEM methodology)

_This example from one Phoenix subarea shows vehicle animation at about 7am._

![http://imageshack.com/a/img571/1856/ihbl.jpg](http://imageshack.com/a/img571/1856/ihbl.jpg)

_This example from one Phoenix subarea shows network volume at about 1pm._

![http://imageshack.com/a/img841/1244/hdkl.jpg](http://imageshack.com/a/img841/1244/hdkl.jpg)

_This example from one Phoenix subarea shows network speed at about 1pm._

![http://imageshack.com/a/img801/4755/tbxy.jpg](http://imageshack.com/a/img801/4755/tbxy.jpg)

_This example from one Phoenix subarea shows network density at about 1pm._

![http://imageshack.com/a/img812/8196/kdi8.jpg](http://imageshack.com/a/img812/8196/kdi8.jpg)

_This example from one Phoenix subarea shows network volume and bottlenecks in Google Earth._

![http://imageshack.com/a/img89/7656/3k7l.jpg](http://imageshack.com/a/img89/7656/3k7l.jpg)

### 2. Spreadsheet based applications for trip generation, trip distribution, and mode choice model calibration ###

A step-by-step tutorial and a self-study software package is provided for students to learn advanced modeling skills.

![http://img51.imageshack.us/img51/6121/excelimg3600.png](http://img51.imageshack.us/img51/6121/excelimg3600.png)
![http://img833.imageshack.us/img833/2350/excelimg2600.png](http://img833.imageshack.us/img833/2350/excelimg2600.png)

Follow a step-by-step tutorial introducing students to the components of the 4-step planning process (trip generation, trip distribution, modal split and traffic assignment) for a small 4-zone transportation network. This Excel spreadsheet application identifies each step's inputs and outputs and allows students to follow a worked example through the first three steps in the 4-step process.

[Download](http://code.google.com/p/nexta/downloads/detail?name=Planning%20Case%20Study%201.1.xlsx&can=2&q=)

### 3. Perform and visualize static and dynamic traffic assignment/simulation results ###

![http://img502.imageshack.us/img502/526/3dcarfollowinganimation.png](http://img502.imageshack.us/img502/526/3dcarfollowinganimation.png)

> _Sioux Falls, South Dakota transportation network:  Utilizing DTALite’s dynamic traffic assignment engine to display simulation results._

DTALite/NEXTA package provides a wide range of data output.
1.	Agent based trajectory
2.	Link-based MOEs, band-width display, user defined offset
3.	Safety and emission statistics based on dynamic assignment results.
4.	Time-dependent path travel times for user-defined path
5.	OD based MOEs
6.	Select link analysis
7.	Select path analysis
8.	Subarea analysis
9.	Summary plot based on a wide range of categories and MOEs


NEXTA provides an excellent multi-project management interface with the following features.
1.	Synchronized display
2.	Click the same location across different networks
3.	Compare link moe across different networks
4.	Find link moe difference between baseline and alternative networks
5.	Vehicle path analysis across different simulation results
6.	Simulation vs. sensor data
7.	Link based comparison
8.	Path travel time comparison
9.	Validation results, diagonal line display to identify outliers


### 4. Import and visualize network-oriented train timetables and provide basic scheduling functionalities. ###

![http://img814.imageshack.us/img814/2619/timetable600.png](http://img814.imageshack.us/img814/2619/timetable600.png)

> _Basic timetable tools in NeXTA for a small network._

### 5. Import multi-day traffic measurement data and provide multi-criteria path finding results (mobility, reliability and emissions) ###

![http://img818.imageshack.us/img818/1407/timedepenentshortestpat.png](http://img818.imageshack.us/img818/1407/timedepenentshortestpat.png)

_This example from the Netherlands shows the shortest, time-dependent least travel time, and economical paths._

http://www.civil.utah.edu/~zhou/NeXTA3Wiki/Multi-day-traffic-MOE.PNG

_This example from the Netherlands shows the multi-day traffic volume and speed measurements, as well as curves for speed-volume-density relationship._





## Related Open-source Projects Developed by Our Team ##

**Smart Routing in the Cloud: Web-based Multi-criteria Routing System based Windows Azure Cloud Computing Platform**
[Smart Routing in the Cloud](http://uroute.org/default.aspx)

**DTALite: Open-source Dynamic Traffic Assignment Model**
[DTALite](https://sites.google.com/site/dtalite/).

## Other Open-source Traffic Simulation/Assignment Systems ##
**TRANSIMS**
http://www.transims-opensource.net

**Aurora Road Network Modeler**
http://code.google.com/p/aurorarnm

**A Traffic Simulation (ATS)**
http://atsimu.sourceforge.net

## Other Transportation Engineering-related Learning Tools ##
**STREET: Simulating Transportation for Realistic Engineering Education and Training**
http://street.umn.edu

## Acknowledgement ##
Berkeley Transportation Systems provided the underlying map image for the
Berkeley Highway Lab network. Caltrans provided sample traffic sensor data.


