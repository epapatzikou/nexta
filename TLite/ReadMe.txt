To Do List.
0. Load image from the scratch, save background information automatically when saving a project
1. Add node, link and vehicle list, link information editing
2. Provide timetabling statistics
3. Estimation and prediction view

Need detailed functional requirements from Jeff.
For transportation planning courses
1. define zones as centroids, use node number
2. OD demand matrix
	2.1 display, 
	2.2 trip generation (export format for trip generation, import data from generation)
	2.3 trip distribution (travel time distance matrix, 3 column form)
	zone aggregation (in the future)
	2.4 traffic assignment results 

Hao: traffic OD estimation
	Given link observation data, generate OD demand flow matrix
	given AVI data and link flow data, generate path flow matrix
	ensemble filtering-based reliable routing algorithm

Tao: traffic prediction engine (based on historical, flow volume), given incident data
	(Xuesong) Based on Newell's model, given different types of data, generate OD demand matrix
	 spatial reliability visualization
		 
    reliable routes, GPS map matching code (in the future)
    
Airline scheduling.
    Lagragian relaxation lower bound, priority upper bound
     