/*******************
FORRWaypoints.h
Implements the FORRwaypoint class, which acts as a grid on top of the map
to count the number of cleaned path points that lie within a sufficiently coarse 
overlay.  These act as pseudo-bases, or "waypoints", which aim to guide the 
robot to useful points on the map that promote fast travel

Written by Matthew Evanusa, November 2014 , Edited by Anoop Aroor March 2017
******************/

#ifndef FORRWAYPOINTS_H
#define FORRWAYPOINTS_H

#include <FORRGeometry.h>
#include "Position.h"

#include <vector>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class FORRWaypoints{

 public:
  
  //Constructor
  FORRWaypoints(double width, double height, double granularity_para){
	granularity = granularity_para;  
  	cout << "Initialize waypoints by setting up the grid"<<endl;
  	boxes_width = width/granularity;
  	boxes_height = height/granularity;
  	map_height = height;
  	map_width = width;
  	for(int i = 0; i <= boxes_width; i++){
    		vector<int> row;
    		for(int j = 0; j <= boxes_height; j++){
      			row.push_back(0);
    		}
    		waypoints.push_back(row);
  	}
  	cout << "Exit setgrid."<<endl;
  	max_grid_value = 0;
  }

  //populate grid from position history
  void populateGridFromPositionHistory(vector<Position> *pos_hist);

  //populate grid from a line segment
  void updateGridFromLine(double x1, double y1, double x2, double y2);

  //outputs to file
  void outputWaypoints(string filename);
  
  //returns the grid overlay postion next to the current position that
  //has the maximum value, given the adjacent non-diagonal positions
  pair<int, int> getNextGridPosition(double curr_x, double curr_y);
  
  //returns the value at the grid [x][y] given map coordinates 
  int getGridValue(double map_x, double map_y);

  int getMaxGridValue();
  
  pair<int,int> convertToGridCoordinates(double x, double y);

  //clears grid after each population to repopulate
  void clearWaypoints();

  int getGranularity(){return granularity;}
  int getMapHeight(){return map_height;}
  int getMapWidth(){return map_width;}
  int getBoxHeight(){return boxes_height;}
  int getBoxWidth(){return boxes_width;}
  vector< vector<int> > getWaypoints(){return waypoints;}

  private:
  	//the grid overlay itself that stores the counter of frequented points along cleaned paths
  	vector< vector<int> > waypoints;
 
  	//variable that allows different sizes of granularity for the overlay
  	//for the future, will be set to the minimum diameter of the regions, 
  	//for now set arbitrarily to 20 distance-units
  	int granularity;
	int max_grid_value;

	int boxes_height, boxes_width;
  	int map_height, map_width;

};


#endif
