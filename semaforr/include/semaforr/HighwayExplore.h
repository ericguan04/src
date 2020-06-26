#ifndef HIGHWAYEXPLORE_H
#define HIGHWAYEXPLORE_H

/**!
  * HighwayExplore.h
  * 
  * /author: Raj Korpan
  *
  *          Construct Highway Graph
  */

#include <FORRGeometry.h>
#include <Position.h>
#include <FORRAction.h>
#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>        //for atan2 and M_PI
#include <algorithm>
#include <map>
#include <set>
#include <numeric>
#include <queue>
#include <sensor_msgs/LaserScan.h>

using namespace std;

struct DecisionPoint{
	Position point;
	sensor_msgs::LaserScan laser;
	double farthest_view_left, farthest_view_middle, farthest_view_right;
	double farthest_angle_left, farthest_angle_middle, farthest_angle_right;
	double farthest_distance_left, farthest_distance_middle, farthest_distance_right;
	double middle_distance, middle_distance_min, left_distance, right_distance, overall_avg_distance, overall_max_distance, overall_min_distance, overall_median_distance, overall_stdev_distance;
	bool direction;
	Position middle_point;
	Position left_point;
	Position right_point;
	DecisionPoint(): point(Position()), laser(sensor_msgs::LaserScan()) { }
	DecisionPoint(Position p, sensor_msgs::LaserScan ls, bool dir = false){
		point = p;
		laser = ls;
		direction = dir;
		double start_angle = ls.angle_min;
		double increment = ls.angle_increment;
		double r_x = p.getX();
		double r_y = p.getY();
		double r_ang = p.getTheta();
		farthest_view_left = 0;
		farthest_view_middle = 329;
		farthest_view_right = 659;
		farthest_distance_left = 0;
		farthest_distance_middle = 0;
		farthest_distance_right = 0;
		middle_distance = 0;
		middle_distance_min = 0;
		left_distance = 0;
		right_distance = 0;
		overall_avg_distance = 0;
		overall_max_distance = 0;
		overall_min_distance = 0;
		overall_median_distance = 0;
		overall_stdev_distance = 0;
		double max_value_left = 0;
		double max_value_middle = 0;
		double max_value_right = 0;
		double min_value_middle = 50;
		double max_value_overall = 0;
		double min_value_overall = 50;
		vector<double> sorted_ranges;
		double left_x = 0, left_y = 0, right_x = 0, right_y = 0, middle_x = 0, middle_y = 0;
		for(int i = 0; i < ls.ranges.size(); i++){
			sorted_ranges.push_back(ls.ranges[i]);
			double angle = start_angle + r_ang;
			if(i >= 580 and i <= 620){
				left_distance += ls.ranges[i];
				if(ls.ranges[i] > max_value_left){
					farthest_distance_left = ls.ranges[i];
					farthest_view_left = i;
					farthest_angle_left = angle;
					max_value_left = ls.ranges[i];
					// left_point = Position(r_x + ls.ranges[i]*cos(angle), r_y + ls.ranges[i]*sin(angle), 0);
				}
				left_x += r_x + ls.ranges[i]*cos(angle);
				left_y += r_y + ls.ranges[i]*sin(angle);
			}
			else if(i >= 40 and i <= 80){
				right_distance += ls.ranges[i];
				if(ls.ranges[i] > max_value_right){
					farthest_distance_right = ls.ranges[i];
					farthest_view_right = i;
					farthest_angle_right = angle;
					max_value_right = ls.ranges[i];
					// right_point = Position(r_x + ls.ranges[i]*cos(angle), r_y + ls.ranges[i]*sin(angle), 0);
				}
				right_x += r_x + ls.ranges[i]*cos(angle);
				right_y += r_y + ls.ranges[i]*sin(angle);
			}
			else if(i >= 195 and i <= 465){
				middle_distance += ls.ranges[i];
				if(ls.ranges[i] > max_value_middle){
					farthest_distance_middle = ls.ranges[i];
					farthest_view_middle = i;
					farthest_angle_middle = angle;
					max_value_middle = ls.ranges[i];
					// middle_point = Position(r_x + ls.ranges[i]*cos(angle), r_y + ls.ranges[i]*sin(angle), 0);
				}
				middle_x += r_x + ls.ranges[i]*cos(angle);
				middle_y += r_y + ls.ranges[i]*sin(angle);
				if(ls.ranges[i] < min_value_middle){
					middle_distance_min = ls.ranges[i];
					min_value_middle = ls.ranges[i];
				}
			}
			start_angle = start_angle + increment;
			overall_avg_distance += ls.ranges[i];
			if(ls.ranges[i] > max_value_overall){
				max_value_overall = ls.ranges[i];
				overall_max_distance = ls.ranges[i];
			}
			if(ls.ranges[i] < min_value_overall){
				min_value_overall = ls.ranges[i];
				overall_min_distance = ls.ranges[i];
			}
		}
		middle_distance = middle_distance / 271.0;
		left_distance = left_distance / 41.0;
		right_distance = right_distance / 41.0;
		overall_avg_distance = overall_avg_distance / ls.ranges.size();
		if(left_x < 0)
			left_x = 0;
		if(left_y < 0)
			left_y = 0;
		if(right_x < 0)
			right_x = 0;
		if(right_y < 0)
			right_y = 0;
		if(middle_x < 0)
			middle_x = 0;
		if(middle_y < 0)
			middle_y = 0;
		left_point = Position(left_x/ 41.0, left_y/ 41.0, 0);
		right_point = Position(right_x/ 41.0, right_y/ 41.0, 0);
		middle_point = Position(middle_x/ 271.0, middle_y/ 271.0, 0);
		sort(sorted_ranges.begin(), sorted_ranges.end());
		overall_median_distance = (sorted_ranges[sorted_ranges.size() / 2 - 1] + sorted_ranges[sorted_ranges.size() / 2]) / 2;
		double sqSum = inner_product(sorted_ranges.begin(), sorted_ranges.end(), sorted_ranges.begin(), 0.0);
        overall_stdev_distance = sqrt(sqSum / sorted_ranges.size() - overall_avg_distance * overall_avg_distance);
	}
	bool operator==(const DecisionPoint p) {
		return (point == p.point);
	}
	bool operator < (const DecisionPoint p) const{
		double dist1 = 0;
		if(direction == true){
			dist1 = right_distance;
		}
		else{
			dist1 = left_distance;
		}
		double dist2 = 0;
		if(p.direction == true){
			dist2 = p.right_distance;
		}
		else{
			dist2 = p.left_distance;
		}
		if(dist1 < dist2)
			return true;
		else
			return false;
		// if(farthest_distance_left < p.farthest_distance_left or farthest_distance_middle < p.farthest_distance_middle or farthest_distance_right < p.farthest_distance_right)
		// 	return true;
		// else
		// 	return false;
	}
};

class Highway{
public:
	Highway(DecisionPoint point){
		highway_points.push_back(point);
		avg_theta = point.point.getTheta();
	}
	vector<DecisionPoint> getHighwayPoints(){return highway_points;}
	int getSizeHighway(){return highway_points.size();}
	double getAvgTheta(){return avg_theta;}

	void addPointToHighway(DecisionPoint new_point){
		std::vector<DecisionPoint>::const_iterator it;
		it = find(highway_points.begin(), highway_points.end(), new_point);
		if(it == highway_points.end()){
			highway_points.push_back(new_point);
		}
		if(highway_points.size() >= 40){
			double avg_s = 0;
			double avg_c = 0;
			for(int i = highway_points.size() - 40; i < highway_points.size(); i++){
				avg_s += sin(highway_points[i].point.getTheta());
				avg_c += cos(highway_points[i].point.getTheta());
			}
			avg_s = avg_s/40.0;
			avg_c = avg_c/40.0;
			avg_theta = atan2(avg_s, avg_c);
		}
		else{
			avg_theta = new_point.point.getTheta();
		}
	}

	DecisionPoint getClosestPointOnHighway(Position point){
		DecisionPoint closest;
		double min_distance = 10000000;
		for(int i = 0; i < highway_points.size(); i++){
			double distance = highway_points[i].point.getDistance(point);
			if(distance < min_distance){
				min_distance = distance;
				closest = highway_points[i];
			}
		}
		return closest;
	}

private:
	vector<DecisionPoint> highway_points;
	double avg_theta;
};

class HighwayExplorer{
public:
	HighwayExplorer(int l, int h, double threshold, double arrMove[], double arrRotate[], int moveArrMax, int rotateArrMax){
		distance_threshold = threshold;
		length = l;
		height = h;
		numMoves = moveArrMax;
		numRotates = rotateArrMax;
		for(int i = 0 ; i < numMoves ; i++) move[i] = arrMove[i];
		for(int i = 0 ; i < numRotates ; i++) rotate[i] = arrRotate[i];
		// cout << "Highway length " << length << " height " << height << endl;
		for(int i = 0; i < l; i++){
			vector<int> col;
			for(int j = 0; j < h; j ++){
				col.push_back(-1);
			}
			highway_grid.push_back(col);
		}
		for(int i = 0; i < l; i++){
			vector<int> col;
			for(int j = 0; j < h; j ++){
				col.push_back(0);
			}
			traveled_grid.push_back(col);
		}
		for(int i = 0; i < l; i++){
			vector< vector< pair<int, int> > > col;
			for(int j = 0; j < h; j ++){
				vector< pair<int, int> > values;
				col.push_back(values);
			}
			highway_grid_connections.push_back(col);
		}
		highways = vector<Highway>();
		highways_complete = false;
		go_to_top_point = false;
		go_to_farthest_on_grid = false;
		too_close = false;
		too_close_front = false;
		top_point_decisions = 0;
		decision_limit = 750;
	};
	~HighwayExplorer(){};

	bool getHighwaysComplete(){return highways_complete;}
	void setHighwaysComplete(double time){
		if(time >= 1200 and time <= 1210){
			highways_complete = true;
			cout << "Highway grid" << endl;
			for(int i = 0; i < highway_grid[0].size(); i++){
				for(int j = 0; j < highway_grid.size(); j++){
					cout << highway_grid[j][i] << " ";
				}
				cout << endl;
			}
		}
	}

	vector< vector<int> > getHighwayGrid(){return highway_grid;}

	vector< vector< vector< pair<int, int> > > > getHighwayGridConnections(){return highway_grid_connections;}

	vector< vector<double> > getHighwayPath(){return path_to_top_point;}

	vector<Position> getHighwayStack(){
		vector<Position> hwst;
		for(int i = 0; i < highway_stack.size(); i++){
			hwst.push_back(highway_stack[i].point);
		}
		return hwst;
	}

	Position getHighwayTarget(){
		if(go_to_top_point == true){
			return top_point.point;
		}
		else{
			return current_target;
		}
	}

	int getLength(){return length;}
	int getHeight(){return height;}

	FORRAction exploreDecision(Position current_point, sensor_msgs::LaserScan current_laser){
		DecisionPoint current_position = DecisionPoint(current_point, current_laser);
		cout << "current_position " << current_position.point.getX() << " " << current_position.point.getY() << " " << current_position.point.getTheta() << " mid avg " << current_position.middle_distance << " mid min " << current_position.middle_distance_min << " mid max " << current_position.farthest_distance_middle << " left avg " << current_position.left_distance << " left max " << current_position.farthest_distance_left << " right avg " << current_position.right_distance << " right max " << current_position.farthest_distance_right << endl;
		// cout << "middle_point " << current_position.middle_point.getX() << " " << current_position.middle_point.getY() << " left_point " << current_position.left_point.getX() << " " << current_position.left_point.getY() << " right_point " << current_position.right_point.getX() << " " << current_position.right_point.getY() << endl;
		if(current_position.right_distance >= distance_threshold){
			DecisionPoint right_position = current_position;
			right_position.direction = true;
			if(pointAlreadyInStack(right_position) == false){
				// cout << "Adding to stack " << highway_stack.size() << " distance " << current_position.right_distance << " view " << current_position.farthest_view_right << endl;
				if(current_position.right_distance >= 2*distance_threshold){
					cout << "Adding to top of stack " << highway_stack.size() << " right distance " << current_position.right_distance << " x " << right_position.point.getX() << " y " << right_position.point.getY() << endl;
					highway_stack.insert(highway_stack.begin(), right_position);
				}
				else{
					cout << "Adding to bottom of stack " << highway_stack.size() << " right distance " << current_position.right_distance << " x " << right_position.point.getX() << " y " << right_position.point.getY() << endl;
					highway_stack.push_back(right_position);
				}
			}
		}
		if(current_position.left_distance >= distance_threshold){
			DecisionPoint left_position = current_position;
			left_position.direction = false;
			if(pointAlreadyInStack(left_position) == false){
				// cout << "Adding to stack " << highway_stack.size() << " distance " << current_position.left_distance << " view " << current_position.farthest_view_left << endl;
				if(current_position.left_distance >= 2*distance_threshold){
					cout << "Adding to top of stack " << highway_stack.size() << " left distance " << current_position.left_distance << " x " << left_position.point.getX() << " y " << left_position.point.getY() << endl;
					highway_stack.insert(highway_stack.begin(), left_position);
				}
				else{
					cout << "Adding to bottom of stack " << highway_stack.size() << " left distance " << current_position.left_distance << " x " << left_position.point.getX() << " y " << left_position.point.getY() << endl;
					highway_stack.push_back(left_position);
				}
			}
		}
		// If the beginning then spin 360 degrees to add to stack
		if(highways.size() == 0){
			last_position = current_position;
			// cout << "last_position " << last_position.point.getX() << " " << last_position.point.getY() << " " << last_position.point.getTheta() << " " << last_highway << endl;
			if(highway_stack.size() == 0){
				return FORRAction(RIGHT_TURN, 1);
			}
			else{
				top_point = highway_stack[0];
				// cout << "Top point " << top_point.point.getX() << " " << top_point.point.getY() << endl;
				highway_stack_completed.push_back(top_point);
				highway_stack.erase(highway_stack.begin());
				Highway new_highway = Highway(top_point);
				highways.push_back(new_highway);
				last_highway = 0;
				top_point_decisions = 0;
				if(top_point.direction == true){
					current_target = top_point.right_point;
					dist_travelled_so_far = 0;
					avg_left = 0;
					avg_right = 0;
					avg_count = 0;
					return goTowardsPoint(current_position, current_target);
				}
				else{
					current_target = top_point.left_point;
					dist_travelled_so_far = 0;
					avg_left = 0;
					avg_right = 0;
					avg_count = 0;
					return goTowardsPoint(current_position, current_target);
				}
			}
		}
		// Once there are items on the stack, pop the top and start to go towards
		// cout << "last_position " << last_position.point.getX() << " " << last_position.point.getY() << " " << last_position.point.getTheta() << " " << last_highway << endl;
		cout << "current_target " << current_target.getX() << " " << current_target.getY() << endl;
		cout << "top_point_decisions " << top_point_decisions << endl;
		cout << "highway_stack.size() " << highway_stack.size() << endl;

		// Check if you can go further, what the width-to-length ratio is, how close the walls
		dist_travelled_so_far += last_position.point.getDistance(current_position.point);
		avg_left += current_position.farthest_distance_left;
		avg_right += current_position.farthest_distance_right;
		avg_count++;
		double width_length_ratio = (current_position.farthest_distance_middle + dist_travelled_so_far) / (avg_left/avg_count + avg_right/avg_count);
		double dist_to_current_target = current_target.getDistance(current_position.point);
		double angle_to_avg_theta = 0;
		if(highways[last_highway].getSizeHighway() >= 40){
			angle_to_avg_theta = highways[last_highway].getAvgTheta() - current_position.point.getTheta();
			if(angle_to_avg_theta > M_PI)
				angle_to_avg_theta = angle_to_avg_theta - (2*M_PI);
			if(angle_to_avg_theta < -M_PI)
				angle_to_avg_theta = angle_to_avg_theta + (2*M_PI);
			angle_to_avg_theta = fabs(angle_to_avg_theta);
		}
		double goal_direction = atan2((current_target.getY() - current_position.point.getY()), (current_target.getX() - current_position.point.getX()));
		double angle_to_current_target = goal_direction - current_position.point.getTheta();
		if(angle_to_current_target > M_PI)
			angle_to_current_target = angle_to_current_target - (2*M_PI);
		if(angle_to_current_target < -M_PI)
			angle_to_current_target = angle_to_current_target + (2*M_PI);
		angle_to_current_target = fabs(angle_to_current_target);
		double middle_direction = atan2((current_position.middle_point.getY() - current_position.point.getY()), (current_position.middle_point.getX() - current_position.point.getX()));
		double angle_to_middle_point = middle_direction - current_position.point.getTheta();
		if(angle_to_middle_point > M_PI)
			angle_to_middle_point = angle_to_middle_point - (2*M_PI);
		if(angle_to_middle_point < -M_PI)
			angle_to_middle_point = angle_to_middle_point + (2*M_PI);
		angle_to_middle_point = fabs(angle_to_middle_point);
		// cout << "dist_travelled_so_far " << dist_travelled_so_far << " avg_left " << avg_left << " avg_right " << avg_right << " avg_count " << avg_count << " width_length_ratio " << width_length_ratio << " dist_to_current_target " << dist_to_current_target << " angle_to_avg_theta " << angle_to_avg_theta << " angle_to_current_target " << angle_to_current_target << endl;
		if((current_position.farthest_distance_middle > last_position.middle_distance or current_position.middle_distance > 0.5) and angle_to_current_target < 0.3490658504 and angle_to_middle_point < 0.3490658504){
			// cout << "More space in front from current position" << endl;
			// Update current target
			current_target = current_position.middle_point;
			// cout << "New current_target " << current_target.getX() << " " << current_target.getY() << endl;
		}

		if(go_to_farthest_on_grid == true and top_point.point.getDistance(current_position.point) <= 0.5){
			top_point_decisions = decision_limit;
			dist_travelled_so_far = 0;
			avg_left = 0;
			avg_right = 0;
			avg_count = 0;
			go_to_top_point = false;
			top_point_decisions = 0;
		}

		if((((width_length_ratio < 1.5 and dist_travelled_so_far > 3.5) or angle_to_avg_theta >= 1.5707963268/2.0 or dist_to_current_target <= 0.5 or current_position.middle_distance <= 0.1) and go_to_top_point == false) or top_point_decisions == decision_limit){
			cout << "Too wide compared to length " << width_length_ratio << " Distance travelled so far " << dist_travelled_so_far << endl;
			cout << "Turn too big " << angle_to_avg_theta << endl;
			cout << "Decision limit reached " << top_point_decisions << endl;
			cout << "Reached current target " << dist_to_current_target << endl;
			cout << "Too close in front " << current_position.middle_distance << endl;
			// Stop current point and go to next on stack
			// cout << "Highway grid" << endl;
			for(int i = 0; i < highway_grid[0].size(); i++){
				for(int j = 0; j < highway_grid.size(); j++){
					if(i == (int)(current_position.point.getY()) and j == (int)(current_position.point.getX())){
						cout << "[" << highway_grid[j][i] << "] "; 
					}
					else{
						cout << highway_grid[j][i] << " ";
					}
				}
				cout << endl;
			}
			dist_travelled_so_far = 0;
			avg_left = 0;
			avg_right = 0;
			avg_count = 0;
			// After finishing point on stack, pop next one
			if(highway_stack.size() > 0){
				// cout << "Going to top point on stack" << endl;
				if((highway_stack[0].direction == true and highway_stack[0].right_distance < 2*distance_threshold) or (highway_stack[0].direction == false and highway_stack[0].left_distance < 2*distance_threshold)){
					sort(highway_stack.begin(), highway_stack.end());
					reverse(highway_stack.begin(), highway_stack.end());
				}
				int start_highway = 0;
				int end_highway = 0;
				int middle_highway = 0;
				bool already_completed = true;
				while((((start_highway >= 0 and end_highway >= 0) or (start_highway >= 0 and middle_highway >= 0) or (middle_highway >= 0 and end_highway >= 0)) or already_completed) and highway_stack.size() > 0){
					top_point = highway_stack[0];
					// cout << "Potential Top point " << top_point.point.getX() << " " << top_point.point.getY() << endl;
					highway_stack.erase(highway_stack.begin());
					start_highway = highway_grid[(int)(top_point.point.getX())][(int)(top_point.point.getY())];
					if(top_point.direction == true){
						end_highway = highway_grid[(int)(top_point.right_point.getX())][(int)(top_point.right_point.getY())];
						middle_highway = highway_grid[(int)((top_point.point.getX() + top_point.right_point.getX())/2.0)][(int)((top_point.point.getY() + top_point.right_point.getY())/2.0)];
					}
					else{
						end_highway = highway_grid[(int)(top_point.left_point.getX())][(int)(top_point.left_point.getY())];
						middle_highway = highway_grid[(int)((top_point.point.getX() + top_point.left_point.getX())/2.0)][(int)((top_point.point.getY() + top_point.left_point.getY())/2.0)];
					}
					if(middle_highway == -2 and end_highway == -2){
						middle_highway = 0;
						end_highway = 0;
					}
					for(int i = 0; i < highway_stack_completed.size(); i++){
						Position new_point_start = top_point.point;
						Position new_point_end;
						Position exist_point_start;
						Position exist_point_end;
						if(top_point.direction == true){
							new_point_end = top_point.right_point;
							exist_point_start = highway_stack_completed[i].point;
							exist_point_end = highway_stack_completed[i].right_point;
						}
						else{
							new_point_end = top_point.left_point;
							exist_point_start = highway_stack_completed[i].point;
							exist_point_end = highway_stack_completed[i].left_point;
						}
						double new_dist = new_point_start.getDistance(new_point_end);
						double exist_dist = exist_point_start.getDistance(exist_point_end);
						double start_to_start_dist = new_point_start.getDistance(exist_point_start);
						double end_to_end_dist = new_point_end.getDistance(exist_point_end);
						double start_to_end_dist = new_point_start.getDistance(exist_point_end);
						double end_to_start_dist = new_point_end.getDistance(exist_point_start);
						// if((start_to_start_dist <= 1 and end_to_end_dist <= 1) or (start_to_end_dist <= 1 and end_to_start_dist <= 1) or (start_to_start_dist <= 1 and start_to_end_dist <= 1) or (end_to_end_dist <= 1 and end_to_start_dist <= 1)){
						// 	already_completed = true;
						// }
						// else{
						// 	already_completed = false;
						// }
						double x = new_point_start.getX();
						double y = new_point_start.getY();
						double x1 = exist_point_start.getX();
						double y1 = exist_point_start.getY();
						double x2 = exist_point_end.getX();
						double y2 = exist_point_end.getY();
						double A = x - x1;
						double B = y - y1;
						double C = x2 - x1;
						double D = y2 - y1;
						double dot = A * C + B * D;
						double len_sq = C * C + D * D;
						double param = -1;
						if (len_sq != 0) //in case of 0 length line
							param = dot / len_sq;
						double xx, yy;
						if (param < 0) {
							xx = x1;
							yy = y1;
						}
						else if (param > 1) {
							xx = x2;
							yy = y2;
						}
						else {
							xx = x1 + param * C;
							yy = y1 + param * D;
						}
						double dx = x - xx;
						double dy = y - yy;
						double start_to_closest_exist = sqrt(dx * dx + dy * dy);
						Position start_to_closest_on_exist(xx, yy, 0);
						x = new_point_end.getX();
						y = new_point_end.getY();
						x1 = exist_point_start.getX();
						y1 = exist_point_start.getY();
						x2 = exist_point_end.getX();
						y2 = exist_point_end.getY();
						A = x - x1;
						B = y - y1;
						C = x2 - x1;
						D = y2 - y1;
						dot = A * C + B * D;
						len_sq = C * C + D * D;
						param = -1;
						if (len_sq != 0) //in case of 0 length line
							param = dot / len_sq;
						if (param < 0) {
							xx = x1;
							yy = y1;
						}
						else if (param > 1) {
							xx = x2;
							yy = y2;
						}
						else {
							xx = x1 + param * C;
							yy = y1 + param * D;
						}
						dx = x - xx;
						dy = y - yy;
						double end_to_closest_exist = sqrt(dx * dx + dy * dy);
						Position end_to_closest_on_exist(xx, yy, 0);
						x = exist_point_start.getX();
						y = exist_point_start.getY();
						x1 = new_point_start.getX();
						y1 = new_point_start.getY();
						x2 = new_point_end.getX();
						y2 = new_point_end.getY();
						A = x - x1;
						B = y - y1;
						C = x2 - x1;
						D = y2 - y1;
						dot = A * C + B * D;
						len_sq = C * C + D * D;
						param = -1;
						if (len_sq != 0) //in case of 0 length line
							param = dot / len_sq;
						if (param < 0) {
							xx = x1;
							yy = y1;
						}
						else if (param > 1) {
							xx = x2;
							yy = y2;
						}
						else {
							xx = x1 + param * C;
							yy = y1 + param * D;
						}
						dx = x - xx;
						dy = y - yy;
						double start_to_closest_new = sqrt(dx * dx + dy * dy);
						Position start_to_closest_on_new(xx, yy, 0);
						x = exist_point_end.getX();
						y = exist_point_end.getY();
						x1 = new_point_start.getX();
						y1 = new_point_start.getY();
						x2 = new_point_end.getX();
						y2 = new_point_end.getY();
						A = x - x1;
						B = y - y1;
						C = x2 - x1;
						D = y2 - y1;
						dot = A * C + B * D;
						len_sq = C * C + D * D;
						param = -1;
						if (len_sq != 0) //in case of 0 length line
							param = dot / len_sq;
						if (param < 0) {
							xx = x1;
							yy = y1;
						}
						else if (param > 1) {
							xx = x2;
							yy = y2;
						}
						else {
							xx = x1 + param * C;
							yy = y1 + param * D;
						}
						dx = x - xx;
						dy = y - yy;
						double end_to_closest_new = sqrt(dx * dx + dy * dy);
						Position end_to_closest_on_new(xx, yy, 0);

						if((end_to_closest_exist <= 1 and start_to_closest_new <= 1 and end_to_closest_on_exist.getDistance(start_to_closest_on_new) >= ((new_dist + exist_dist)/2.0)/3.0) or (end_to_closest_new <= 1 and start_to_closest_exist <= 1 and end_to_closest_on_new.getDistance(start_to_closest_on_exist) >= ((new_dist + exist_dist)/2.0)/3.0) or (start_to_start_dist <= 1 and end_to_closest_exist <= 1) or (start_to_start_dist <= 1 and end_to_closest_new <= 1) or (start_to_closest_exist <= 1 and end_to_closest_exist <= 1) or (start_to_closest_new <= 1 and end_to_closest_new <= 1) or (end_to_end_dist <= 1 and start_to_closest_exist <= 1) or (end_to_end_dist <= 1 and start_to_closest_new <= 1) or (start_to_start_dist <= 1 and end_to_end_dist <= 1) or (start_to_end_dist <= 1 and end_to_start_dist <= 1) or (start_to_start_dist <= 1 and start_to_end_dist <= 1) or (end_to_end_dist <= 1 and end_to_start_dist <= 1)){
							already_completed = true;
							break;
						}
						else{
							already_completed = false;
						}
					}
					highway_stack_completed.push_back(top_point);
					// cout << "start_highway " << start_highway  << " middle_highway " << middle_highway << " end_highway " << end_highway << endl;
				}
				// cout << "Final Top point " << top_point.point.getX() << " " << top_point.point.getY() << endl;
				Highway new_highway = Highway(top_point);
				highways.push_back(new_highway);
				last_highway = last_highway + 1;
				top_point_decisions = 0;
				go_to_top_point = true;
				path_to_top_point.clear();
				go_to_farthest_on_grid = false;
			}
			else if(go_to_farthest_on_grid == false){
				double dist_to_farthest = 0;
				for(int i = 0; i < highway_grid.size(); i++){
					for(int j = 0; j < highway_grid[i].size(); j++){
						if(highway_grid[i][j] >= 0){
							double dist_to_grid = current_position.point.getDistance(Position(i,j,0));
							if(dist_to_grid > dist_to_farthest){
								top_point = highways[highway_grid[i][j]].getClosestPointOnHighway(Position(i,j,0));
							}
						}
					}
				}
				top_point_decisions = 0;
				go_to_top_point = true;
				path_to_top_point.clear();
				go_to_farthest_on_grid = true;
			}
			else{
				// cout << "No more in stack" << endl;
				if(highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] >= 0){
					highways[highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())]].addPointToHighway(current_position);
				}
				last_position = current_position;
				go_to_top_point = false;
				highways_complete = true;
				return FORRAction(FORWARD, 0);
			}
		}

		if(current_position.left_distance <= 0.2 or current_position.right_distance <= 0.2){
			cout << "Too close on sides, turn away" << endl;
			// Move away from sides
			if(highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] >= 0){
				highways[highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())]].addPointToHighway(current_position);
			}
			// if(last_position.left_distance <= 0.15 or last_position.right_distance <= 0.15){
			// 	last_position = current_position;
			// 	top_point_decisions++;
			// 	return FORRAction(FORWARD, 3);
			// }
			// else 
			if(current_position.left_distance <= 0.2){
				// last_position = current_position;
				top_point_decisions++;
				// return FORRAction(RIGHT_TURN, 2);
				return goTowardsPoint(current_position, current_position.right_point);
			}
			else if(current_position.right_distance <= 0.2){
				// last_position = current_position;
				top_point_decisions++;
				// return FORRAction(LEFT_TURN, 2);
				return goTowardsPoint(current_position, current_position.left_point);
			}
		}
		else if(go_to_top_point == true){
			// cout << "Go to top point" << endl;
			if(highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] >= 0){
				highways[highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())]].addPointToHighway(current_position);
			}
			cout << "Top point " << top_point.point.getX() << " " << top_point.point.getY() << endl;
			double dist_to_top_point = top_point.point.getDistance(current_position.point);
			// cout << "Distance to top point " << dist_to_top_point << " current theta " << current_position.point.getTheta() << " top point angles " << top_point.farthest_angle_left << " " << top_point.farthest_angle_right << endl;
			if(dist_to_top_point <= 0.5){
				cout << "Top point achieved, turn towards stretch" << endl;
				dist_travelled_so_far = 0;
				avg_left = 0;
				avg_right = 0;
				avg_count = 0;
				go_to_top_point = false;
				top_point_decisions = 0;
				if(top_point.direction == true){
					current_target = top_point.right_point;
					return goTowardsPoint(current_position, current_target);
				}
				else{
					current_target = top_point.left_point;
					return goTowardsPoint(current_position, current_target);
				}
			}
			else if(dist_to_top_point <= 1){
				cout << "Top point close, go towards" << endl;
				top_point_decisions++;
				return goTowardsPoint(current_position, top_point.point);
			}
			else if(path_to_top_point.size() > 0){
				cout << "Top point not in range, go to top point by following path" << endl;
				waypointAchieved(current_position);
				if(path_to_top_point.size() == 0){
					last_position = current_position;
					top_point_decisions++;
					return FORRAction(FORWARD, 0);
				}
				else if(top_point_decisions % 100 == 0 and path_to_top_point.size() >= 2){
					// cout << "Erase current waypoint and assign next one" << endl;
					path_to_top_point.erase(path_to_top_point.begin());
					// cout << "New waypoint " << path_to_top_point[0][0] << " " << path_to_top_point[0][1] << endl;
					top_point_decisions++;
					return goTowardsPoint(current_position, Position(path_to_top_point[0][0], path_to_top_point[0][1], 0));
				}
				else{
					// cout << "Current waypoint " << path_to_top_point[0][0] << " " << path_to_top_point[0][1] << endl;
					top_point_decisions++;
					return goTowardsPoint(current_position, Position(path_to_top_point[0][0], path_to_top_point[0][1], 0));
				}
			}
			else{
				findPathOnGrid(current_position, top_point);
				// cout << "Current waypoint " << path_to_top_point[0][0] << " " << path_to_top_point[0][1] << endl;
				top_point_decisions++;
				return goTowardsPoint(current_position, Position(path_to_top_point[0][0], path_to_top_point[0][1], 0));
			}
		}
		else{
			// cout << "Going to current_target " << current_target.getX() << " " << current_target.getY() << endl;
			highways[last_highway].addPointToHighway(current_position);
			// cout << "Current grid value " << highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] << endl;
			if(highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] == -1){
				highway_grid[(int)(current_position.point.getX())][(int)(current_position.point.getY())] = last_highway;
				double multiplier = 10;
				vector< vector<int> > passed_grid;
				for(int i = 0; i < length*multiplier; i++){
					vector<int> col;
					for(int j = 0; j < height*multiplier; j ++){
						col.push_back(0);
					}
					passed_grid.push_back(col);
				}
				vector< vector<int> > hit_grid;
				for(int i = 0; i < length*multiplier; i++){
					vector<int> col;
					for(int j = 0; j < height*multiplier; j ++){
						col.push_back(0);
					}
					hit_grid.push_back(col);
				}
				int startx = (int)(current_position.point.getX()*multiplier);
				int starty = (int)(current_position.point.getY()*multiplier);
				int rightx = (int)(current_position.right_point.getX()*multiplier);
				int righty = (int)(current_position.right_point.getY()*multiplier);
				int leftx = (int)(current_position.left_point.getX()*multiplier);
				int lefty = (int)(current_position.left_point.getY()*multiplier);
				double righta = starty - righty;
				double rightb = rightx - startx;
				double rightc = (startx - rightx) * starty + (righty - starty) * startx;
				double lefta = starty - lefty;
				double leftb = leftx - startx;
				double leftc = (startx - leftx) * starty + (lefty - starty) * startx;
				if(startx > rightx){
					for(int i = startx; i > rightx; i--){
						passed_grid[i][-(righta * i + rightc) / rightb] = passed_grid[i][-(righta * i + rightc) / rightb] + 1;
					}
				}
				else if(startx < rightx){
					for(int i = startx; i < rightx; i++){
						passed_grid[i][-(righta * i + rightc) / rightb] = passed_grid[i][-(righta * i + rightc) / rightb] + 1;
					}
				}
				else{
					if(starty > righty){
						for(int i = starty; i > righty; i--){
							passed_grid[-(rightb * i + rightc) / righta][i] = passed_grid[-(rightb * i + rightc) / righta][i] + 1;
						}
					}
					else if(starty < righty){
						for(int i = starty; i < righty; i++){
							passed_grid[-(rightb * i + rightc) / righta][i] = passed_grid[-(rightb * i + rightc) / righta][i] + 1;
						}
					}
					else{
						passed_grid[startx][starty] = passed_grid[startx][starty] + 1;
					}
				}
				if(startx > leftx){
					for(int i = startx; i > leftx; i--){
						passed_grid[i][-(lefta * i + leftc) / leftb] = passed_grid[i][-(lefta * i + leftc) / leftb] + 1;
					}
				}
				else if(startx < leftx){
					for(int i = startx; i < leftx; i++){
						passed_grid[i][-(lefta * i + leftc) / leftb] = passed_grid[i][-(lefta * i + leftc) / leftb] + 1;
					}
				}
				else{
					if(starty > lefty){
						for(int i = starty; i > lefty; i--){
							passed_grid[-(leftb * i + leftc) / lefta][i] = passed_grid[-(leftb * i + leftc) / lefta][i] + 1;
						}
					}
					else if(starty < lefty){
						for(int i = starty; i < lefty; i++){
							passed_grid[-(leftb * i + leftc) / lefta][i] = passed_grid[-(leftb * i + leftc) / lefta][i] + 1;
						}
					}
					else{
						passed_grid[startx][starty] = passed_grid[startx][starty] + 1;
					}
				}
				hit_grid[rightx][righty] = 1;
				hit_grid[leftx][lefty] = 1;
				vector< vector<double> > ratio_grid;
				for(int i = 0; i < length*multiplier; i++){
					vector<double> col;
					for(int j = 0; j < height*multiplier; j ++){
						col.push_back(0.0);
					}
					ratio_grid.push_back(col);
				}
				for(int i = 0; i < passed_grid.size(); i++){
					for(int j = 0; j < passed_grid[i].size(); j++){
						if(passed_grid[i][j] > 0 or hit_grid[i][j] > 0){
							ratio_grid[i][j] = (double)(hit_grid[i][j]) / ((double)(hit_grid[i][j]) + (double)(passed_grid[i][j]));
						}
						else{
							ratio_grid[i][j] = -1;
						}
					}
				}
				vector< vector<double> > count_grid;
				for(int i = 0; i < length; i++){
					vector<double> col;
					for(int j = 0; j < height; j ++){
						col.push_back(0.0);
					}
					count_grid.push_back(col);
				}
				vector< vector<double> > ratio_sum_grid;
				for(int i = 0; i < length; i++){
					vector<double> col;
					for(int j = 0; j < height; j ++){
						col.push_back(0.0);
					}
					ratio_sum_grid.push_back(col);
				}
				vector< vector<double> > distance_sum_grid;
				for(int i = 0; i < length; i++){
					vector<double> col;
					for(int j = 0; j < height; j ++){
						col.push_back(0.0);
					}
					distance_sum_grid.push_back(col);
				}
				for(int i = 0; i < ratio_grid.size(); i++){
					for(int j = 0; j < ratio_grid[i].size(); j++){
						if(ratio_grid[i][j] >= 0){
							int indx = (int)((double)(i)/multiplier);
							int indy = (int)((double)(j)/multiplier);
							count_grid[indx][indy] = count_grid[indx][indy] + 1;
							ratio_sum_grid[indx][indy] = ratio_sum_grid[indx][indy] + ratio_grid[i][j];
							distance_sum_grid[indx][indy] = distance_sum_grid[indx][indy] + sqrt((double)((i-startx)*(i-startx)+(j-starty)*(j-starty)))/multiplier;
						}
					}
				}
				vector< vector<double> > ratio_avg_grid;
				for(int i = 0; i < length; i++){
					vector<double> col;
					for(int j = 0; j < height; j ++){
						col.push_back(0.0);
					}
					ratio_avg_grid.push_back(col);
				}
				vector< vector<double> > distance_avg_grid;
				for(int i = 0; i < length; i++){
					vector<double> col;
					for(int j = 0; j < height; j ++){
						col.push_back(0.0);
					}
					distance_avg_grid.push_back(col);
				}
				for(int i = 0; i < count_grid.size(); i++){
					for(int j = 0; j < count_grid[i].size(); j++){
						if(count_grid[i][j] > 0){
							ratio_avg_grid[i][j] = ratio_sum_grid[i][j] / count_grid[i][j];
							distance_avg_grid[i][j] = distance_sum_grid[i][j] / count_grid[i][j];
						}
						else{
							ratio_avg_grid[i][j] = -1;
							distance_avg_grid[i][j] = -1;
						}
					}
				}
				for(int i = 0; i < ratio_avg_grid.size(); i++){
					for(int j = 0; j < ratio_avg_grid[i].size(); j++){
						if(distance_avg_grid[i][j] <= 2 and ratio_avg_grid[i][j] <= 0.5 and distance_avg_grid[i][j] >= 0 and ratio_avg_grid[i][j] >= 0){
							highway_grid[i][j] = last_highway;
							pair<int, int> connection_ab (i, j);
							highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].push_back(connection_ab);
							pair<int, int> connection_ba ((int)(current_position.point.getX()), (int)(current_position.point.getY()));
							highway_grid_connections[i][j].push_back(connection_ba);
						}
						else if(distance_avg_grid[i][j] <= 4 and ratio_avg_grid[i][j] > 0.5 and distance_avg_grid[i][j] >= 0 and ratio_avg_grid[i][j] >= 0 and highway_grid[i][j] < 0){
							highway_grid[i][j] = -2;
						}
					}
				}
				// if(current_position.left_distance >= 0.1){
				// 	double step_size = 0.01;
				// 	double tx,ty;
				// 	vector < vector <int> > neighbor_grids;
				// 	vector <double> neighbor_distances;
				// 	for(double step = 0; step <= 1; step += step_size){
				// 		tx = (current_position.left_point.getX() * step) + (current_position.point.getX() * (1-step));
				// 		ty = (current_position.left_point.getY() * step) + (current_position.point.getY() * (1-step));
				// 		double dist_to_current_from_t = current_position.point.getDistance(Position(tx, ty, 0));
				// 		vector<double> neighbor;
				// 		neighbor.push_back((int)(tx));
				// 		neighbor.push_back((int)(ty));
				// 		if(find(neighbor_grids.begin(), neighbor_grids.end(), neighbor) != neighbor_grids.end()){
				// 			continue;
				// 		}
				// 		else{
				// 			neighbor_grids.push_back(neighbor);
				// 			neighbor_distances.push_back(dist_to_current_from_t);
				// 		}
				// 	}
				// 	if(neighbor_grids.size() == 1){
				// 		continue;
				// 	}
				// 	else if(neighbor_grids.size() == 2){
				// 		if(highway_grid[neighbor_grids[1][0]][neighbor_grids[1][1]] == -1){
				// 			highway_grid[neighbor_grids[1][0]][neighbor_grids[1][1]] = -2;
				// 		}
				// 	}

				// 	for(int i = 1; i < neighbor_grids.size()-1; i++){
				// 		if(neighbor_grids[i][0] != neighbor_grids[i-1][0] and neighbor_grids[i][1] != neighbor_grids[i-1][1] and neighbor_distances[1] <= 2 and neighbor_distances[1] <= current_position.left_distance and highway_grid[neighbor_grids[i][0]][neighbor_grids[i][1]] == -1){
				// 			highway_grid[neighbor_grids[i][0]][neighbor_grids[i][1]] = last_highway;
				// 			pair<int, int> connection_ab (neighbor_grids[i][0], neighbor_grids[i][1]);
				// 			highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].push_back(connection_ab);
				// 			pair<int, int> connection_ba ((int)(current_position.point.getX()), (int)(current_position.point.getY()));
				// 			highway_grid_connections[neighbor_grids[i][0]][neighbor_grids[i][1]].push_back(connection_ba);
				// 		}
				// 	}
				// 	// double step_size = 0.1;
				// 	// double tx,ty;
				// 	// for(double step = 0; step <= 1; step += step_size){
				// 	// 	tx = (current_position.left_point.getX() * step) + (current_position.point.getX() * (1-step));
				// 	// 	ty = (current_position.left_point.getY() * step) + (current_position.point.getY() * (1-step));
				// 	// 	if(current_position.point.getDistance(Position(tx, ty, 0)) <= 2 and current_position.point.getDistance(Position(tx, ty, 0)) <= current_position.left_distance){
				// 	// 		if(highway_grid[(int)(tx)][(int)(ty)] == -1){
				// 	// 			highway_grid[(int)(tx)][(int)(ty)] = last_highway;
				// 	// 			pair<int, int> connection_ab ((int)(tx), (int)(ty));
				// 	// 			highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].push_back(connection_ab);
				// 	// 			pair<int, int> connection_ba ((int)(current_position.point.getX()), (int)(current_position.point.getY()));
				// 	// 			highway_grid_connections[(int)(tx)][(int)(ty)].push_back(connection_ba);
				// 	// 		}
				// 	// 	}
				// 	// }
				// }
				// if(current_position.right_distance >= 2){
				// 	double step_size = 0.1;
				// 	double tx,ty;
				// 	for(double step = 0; step <= 1; step += step_size){
				// 		tx = (current_position.right_point.getX() * step) + (current_position.point.getX() * (1-step));
				// 		ty = (current_position.right_point.getY() * step) + (current_position.point.getY() * (1-step));
				// 		if(current_position.point.getDistance(Position(tx, ty, 0)) <= 2 and current_position.point.getDistance(Position(tx, ty, 0)) <= current_position.right_distance){
				// 			if(highway_grid[(int)(tx)][(int)(ty)] == -1){
				// 				highway_grid[(int)(tx)][(int)(ty)] = last_highway;
				// 				pair<int, int> connection_ab ((int)(tx), (int)(ty));
				// 				highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].push_back(connection_ab);
				// 				pair<int, int> connection_ba ((int)(current_position.point.getX()), (int)(current_position.point.getY()));
				// 				highway_grid_connections[(int)(tx)][(int)(ty)].push_back(connection_ba);
				// 			}
				// 		}
				// 	}
				// }
			}
			if((int)(current_position.point.getX()) != (int)(last_position.point.getX()) or (int)(current_position.point.getY()) != (int)(last_position.point.getY())){
				pair<int, int> connection_ab ((int)(last_position.point.getX()), (int)(last_position.point.getY()));
				std::vector< pair<int, int> >::const_iterator it;
				it = find(highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].begin(), highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].end(), connection_ab);
				if(it == highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].end()){
					highway_grid_connections[(int)(current_position.point.getX())][(int)(current_position.point.getY())].push_back(connection_ab);
				}
				pair<int, int> connection_ba ((int)(current_position.point.getX()), (int)(current_position.point.getY()));
				it = find(highway_grid_connections[(int)(last_position.point.getX())][(int)(last_position.point.getY())].begin(), highway_grid_connections[(int)(last_position.point.getX())][(int)(last_position.point.getY())].end(), connection_ba);
				if(it == highway_grid_connections[(int)(last_position.point.getX())][(int)(last_position.point.getY())].end()){
					highway_grid_connections[(int)(last_position.point.getX())][(int)(last_position.point.getY())].push_back(connection_ba);
				}
			}
			return goTowardsPoint(current_position, current_target);
		}
	}

	void findPathOnGrid(DecisionPoint current_point, DecisionPoint target_point){
		int current_x = (int)(current_point.point.getX());
		int current_y = (int)(current_point.point.getY());
		int target_x = (int)(target_point.point.getX());
		int target_y = (int)(target_point.point.getY());
		path_to_top_point.clear();
		// cout << "Current point coordinates in grid " << current_x << " " << current_y << " target_point " << target_x << " " << target_y << endl;
		// cout << "Current highway " << highway_grid[current_x][current_y] << " Target highway " << highway_grid[target_x][target_y] << endl;
		if(current_x == target_x and current_y == target_y){
			vector<double> current;
			current.push_back(target_point.point.getX());
			current.push_back(target_point.point.getY());
			path_to_top_point.push_back(current);
			return;
		}
		traveled_grid.clear();
		for(int i = 0; i < length; i++){
			vector<int> col;
			for(int j = 0; j < height; j ++){
				col.push_back(0);
			}
			traveled_grid.push_back(col);
		}
		if(highway_grid[current_x][current_y] < 0){
			double min_dist = 10000000;
			DecisionPoint closest_to_current = current_point;
			for(int i = 0; i < highways.size(); i++){
				DecisionPoint close_point = highways[i].getClosestPointOnHighway(current_point.point);
				if(close_point.point.getDistance(current_point.point) < min_dist and highway_grid[(int)(close_point.point.getX())][(int)(close_point.point.getY())] >= 0){
					min_dist = close_point.point.getDistance(current_point.point);
					closest_to_current = close_point;
				}
			}
			// cout << "Closest to current " << (int)(closest_to_current.point.getX()) << " " << (int)(closest_to_current.point.getY()) << endl;
			current_x = (int)(closest_to_current.point.getX());
			current_y = (int)(closest_to_current.point.getY());
		}
		if(highway_grid[target_x][target_y] < 0){
			double min_dist = 10000000;
			DecisionPoint closest_to_current = target_point;
			for(int i = 0; i < highways.size(); i++){
				DecisionPoint close_point = highways[i].getClosestPointOnHighway(target_point.point);
				if(close_point.point.getDistance(target_point.point) < min_dist and highway_grid[(int)(close_point.point.getX())][(int)(close_point.point.getY())] >= 0){
					min_dist = close_point.point.getDistance(target_point.point);
					closest_to_current = close_point;
				}
			}
			// cout << "Closest to target " << (int)(closest_to_current.point.getX()) << " " << (int)(closest_to_current.point.getY()) << endl;
			target_x = (int)(closest_to_current.point.getX());
			target_y = (int)(closest_to_current.point.getY());
		}
		traveled_grid[current_x][current_y] = 1;
		vector< vector<int> > added_points;
		vector< vector<int> > previous_cell;
		for(int i = 0; i < highway_grid_connections[current_x][current_y].size(); i++){
			int c_x = highway_grid_connections[current_x][current_y][i].first;
			int c_y = highway_grid_connections[current_x][current_y][i].second;
			if(traveled_grid[c_x][c_y] != 1){
				traveled_grid[c_x][c_y] = 1;
				vector<int> new_id;
				new_id.push_back(c_x);
				new_id.push_back(c_y);
				added_points.push_back(new_id);
				new_id.push_back(current_x);
				new_id.push_back(current_y);
				previous_cell.push_back(new_id);
				// cout << "Neighbor " << c_x << " " << c_y << " Previous " << current_x << " " << current_y << endl;
			}
		}
		// cout << "Added points " << added_points.size() << endl;
		while(added_points.size() > 0){
			int new_x = added_points[0][0];
			int new_y = added_points[0][1];
			if(new_x == target_x and new_y == target_y){
				// cout << "Found target point" << endl;
				break;
			}
			added_points.erase(added_points.begin());
			for(int i = 0; i < highway_grid_connections[new_x][new_y].size(); i++){
				int c_x = highway_grid_connections[new_x][new_y][i].first;
				int c_y = highway_grid_connections[new_x][new_y][i].second;
				if(traveled_grid[c_x][c_y] != 1){
					traveled_grid[c_x][c_y] = 1;
					vector<int> new_id;
					new_id.push_back(c_x);
					new_id.push_back(c_y);
					added_points.push_back(new_id);
					new_id.push_back(new_x);
					new_id.push_back(new_y);
					previous_cell.push_back(new_id);
					// cout << "Neighbor " << c_x << " " << c_y << " Previous " << new_x << " " << new_y << endl;
				}
			}
			// cout << "Added points " << added_points.size() << endl;
		}
		// cout << "Previous cell " << previous_cell.size() << endl;
		vector< vector<int> > path;
		if(traveled_grid[target_x][target_y] == 1){
			vector<int> target;
			target.push_back(target_x);
			target.push_back(target_y);
			path.insert(path.begin(), target);
			// cout << "Path " << path.size() << " target " << target_x << " " << target_y << endl;
			bool found = true;
			while(found){
				int point_x = path[0][0];
				int point_y = path[0][1];
				found = false;
				// cout << "Path " << path.size() << " point " << point_x << " " << point_y << endl; 
				for(int i = 0; i < previous_cell.size(); i++){
					if(previous_cell[i][0] == point_x and previous_cell[i][1] == point_y){
						// cout << "Match " << previous_cell[i][0] << " " << previous_cell[i][1] << endl;
						vector<int> new_target;
						new_target.push_back(previous_cell[i][2]);
						new_target.push_back(previous_cell[i][3]);
						path.insert(path.begin(), new_target);
						// cout << "Previous " << previous_cell[i][2] << " " << previous_cell[i][3] << endl;
						found = true;
						break;
					}
				}
				// cout << path[0][0] << " " << current_x << " " << path[0][1] << " " << current_y << endl;
				if(path[0][0] == current_x and path[0][1] == current_y){
					break;
				}
			}
			vector< vector<double> > path_from_highway;
			// cout << "Path " << path.size() << endl;
			for(int i = 0; i < path.size(); i++){
				// cout << path[i][0] << " " << path[i][1] << " " << highway_grid[path[i][0]][path[i][1]] << endl;
				if(highway_grid[path[i][0]][path[i][1]] >= 0){
					if(i > 0){
						DecisionPoint closest_prev = highways[highway_grid[path[i][0]][path[i][1]]].getClosestPointOnHighway(Position(path[i-1][0], path[i-1][1], 0));
						// cout << closest_prev.point.getX() << " " << closest_prev.point.getY() << endl;
						vector<double> closest_point_prev;
						closest_point_prev.push_back(closest_prev.point.getX());
						closest_point_prev.push_back(closest_prev.point.getY());
						path_from_highway.push_back(closest_point_prev);
					}
					DecisionPoint closest = highways[highway_grid[path[i][0]][path[i][1]]].getClosestPointOnHighway(Position(path[i][0], path[i][1], 0));
					// cout << closest.point.getX() << " " << closest.point.getY() << endl;
					vector<double> closest_point;
					closest_point.push_back(closest.point.getX());
					closest_point.push_back(closest.point.getY());
					path_from_highway.push_back(closest_point);
				}
			}
			vector<double> current;
			current.push_back(target_point.point.getX());
			current.push_back(target_point.point.getY());
			path_from_highway.push_back(current);
			// path_to_top_point = path;
			path_to_top_point = path_from_highway;
		}
		else{
			vector< vector<double> > path_from_highway;
			vector<double> current;
			current.push_back(target_point.point.getX());
			current.push_back(target_point.point.getY());
			path_from_highway.push_back(current);
			// path_to_top_point = path;
			path_to_top_point = path_from_highway;
		}
	}

	void waypointAchieved(DecisionPoint current_position){
		// cout << "In waypointAchieved" << endl;
		bool erase_waypoint = true;
		while(erase_waypoint){
			Position current_waypoint = Position(path_to_top_point[0][0], path_to_top_point[0][1], 0);
			double dist_to_current_waypoint = current_position.point.getDistance(current_waypoint);
			if(dist_to_current_waypoint <= 0.25){
				// cout << "Waypoint Achieved, removing waypoint" << endl;
				path_to_top_point.erase(path_to_top_point.begin());
			}
			else{
				erase_waypoint = false;
			}
		}
	}

	FORRAction goTowardsPoint(DecisionPoint current_position, Position target_position){
		// cout << "In goTowardsPoint" << endl;
		double distance_from_target = current_position.point.getDistance(target_position);
		// cout << "Distance from target : " << distance_from_target << endl;

		// compute the angular difference between the direction to the target and the current robot direction
		double robot_direction = current_position.point.getTheta();
		double goal_direction = atan2((target_position.getY() - current_position.point.getY()), (target_position.getX() - current_position.point.getX()));
		double required_rotation = goal_direction - robot_direction;

		if(required_rotation > M_PI)
			required_rotation = required_rotation - (2*M_PI);
		if(required_rotation < -M_PI)
			required_rotation = required_rotation + (2*M_PI);
		// cout << "Robot direction : " << robot_direction << ", Goal Direction : " << goal_direction << ", Required rotation : " << required_rotation << endl;
		// if the angular difference is greater than smallest turn possible 
		// pick the right turn to allign itself to the target

		FORRAction decision;
		int rotIntensity=0;
		while(fabs(required_rotation) > rotate[rotIntensity] and rotIntensity < numRotates) {
			rotIntensity++;
		}
		// cout << "Rotation Intensity : " << rotIntensity << endl;
		if (rotIntensity > 1) {
			if (required_rotation < 0){
				decision = FORRAction(RIGHT_TURN, rotIntensity-1);
			}
			else {
				decision = FORRAction(LEFT_TURN, rotIntensity-1);
			}
		}
		else {
			decision = FORRAction(FORWARD, 4);
			int intensity=0;
			while(distance_from_target > move[intensity] and intensity < numMoves) {
				intensity++;
			}
			if(intensity > 1)
				intensity--;
			// cout << "Move Intensity : " << intensity << endl;

			while(move[intensity] > current_position.middle_distance_min){
				intensity--;
			}
			// cout << "Move Intensity : " << intensity << endl;
			if(go_to_top_point == true and intensity > 0)
				decision = FORRAction(FORWARD, intensity);
		}
		// cout << "Action choosen : " << decision.type << "," << decision.parameter << endl;
		last_position = current_position;
		return decision;
	}

	bool pointAlreadyInStack(DecisionPoint new_point){
		cout << "Check if point in stack" << endl;
		int start_highway = highway_grid[(int)(new_point.point.getX())][(int)(new_point.point.getY())];
		int end_highway = -1;
		int middle_highway = -1;
		double width_length = 0;
		if(new_point.direction == true){
			end_highway = highway_grid[(int)(new_point.right_point.getX())][(int)(new_point.right_point.getY())];
			middle_highway = highway_grid[(int)((new_point.point.getX() + new_point.right_point.getX())/2.0)][(int)((new_point.point.getY() + new_point.right_point.getY())/2.0)];
			width_length = new_point.farthest_distance_right / (new_point.farthest_distance_middle);
		}
		else{
			end_highway = highway_grid[(int)(new_point.left_point.getX())][(int)(new_point.left_point.getY())];
			middle_highway = highway_grid[(int)((new_point.point.getX() + new_point.left_point.getX())/2.0)][(int)((new_point.point.getY() + new_point.left_point.getY())/2.0)];
			width_length = new_point.farthest_distance_left / (new_point.farthest_distance_middle);
		}
		cout << "width_length " << width_length << " overall_avg_distance " << new_point.overall_avg_distance << " overall_max_distance " << new_point.overall_max_distance << " overall_min_distance " << new_point.overall_min_distance << " overall_median_distance " << new_point.overall_median_distance << " overall_stdev_distance " << new_point.overall_stdev_distance << endl;
		if(width_length < 1){
			return true;
		}
		if(new_point.overall_avg_distance > 4.75 and new_point.overall_max_distance <= 16.5 and new_point.overall_median_distance > 3.9 and new_point.overall_stdev_distance <= 4.25){
			return true;
		}
		else if(new_point.overall_avg_distance > 4.3 and new_point.overall_max_distance <= 16.5 and new_point.overall_median_distance <= 3.9 and new_point.overall_min_distance > 1.6){
			return true;
		}
		bool close_to_existing = false;
		bool close_to_completed = false;
		for(int i = 0; i < highway_stack.size(); i++){
			Position new_point_start = new_point.point;
			Position new_point_end;
			Position exist_point_start;
			Position exist_point_end;
			if(new_point.direction == true){
				new_point_end = new_point.right_point;
				exist_point_start = highway_stack[i].point;
				exist_point_end = highway_stack[i].right_point;
			}
			else{
				new_point_end = new_point.left_point;
				exist_point_start = highway_stack[i].point;
				exist_point_end = highway_stack[i].left_point;
			}
			double new_dist = new_point_start.getDistance(new_point_end);
			double exist_dist = exist_point_start.getDistance(exist_point_end);
			double start_to_start_dist = new_point_start.getDistance(exist_point_start);
			double end_to_end_dist = new_point_end.getDistance(exist_point_end);
			double start_to_end_dist = new_point_start.getDistance(exist_point_end);
			double end_to_start_dist = new_point_end.getDistance(exist_point_start);
			if((start_to_start_dist <= 1 and end_to_end_dist <= 1) or (start_to_end_dist <= 1 and end_to_start_dist <= 1) or (start_to_start_dist <= 1 and start_to_end_dist <= 1) or (end_to_end_dist <= 1 and end_to_start_dist <= 1)){
				close_to_existing = true;
				break;
			}
			double x = new_point_start.getX();
			double y = new_point_start.getY();
			double x1 = exist_point_start.getX();
			double y1 = exist_point_start.getY();
			double x2 = exist_point_end.getX();
			double y2 = exist_point_end.getY();
			double A = x - x1;
			double B = y - y1;
			double C = x2 - x1;
			double D = y2 - y1;
			double dot = A * C + B * D;
			double len_sq = C * C + D * D;
			double param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			double xx, yy;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			double dx = x - xx;
			double dy = y - yy;
			double start_to_closest_exist = sqrt(dx * dx + dy * dy);
			Position start_to_closest_on_exist(xx, yy, 0);
			x = new_point_end.getX();
			y = new_point_end.getY();
			x1 = exist_point_start.getX();
			y1 = exist_point_start.getY();
			x2 = exist_point_end.getX();
			y2 = exist_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double end_to_closest_exist = sqrt(dx * dx + dy * dy);
			Position end_to_closest_on_exist(xx, yy, 0);
			x = exist_point_start.getX();
			y = exist_point_start.getY();
			x1 = new_point_start.getX();
			y1 = new_point_start.getY();
			x2 = new_point_end.getX();
			y2 = new_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double start_to_closest_new = sqrt(dx * dx + dy * dy);
			Position start_to_closest_on_new(xx, yy, 0);
			x = exist_point_end.getX();
			y = exist_point_end.getY();
			x1 = new_point_start.getX();
			y1 = new_point_start.getY();
			x2 = new_point_end.getX();
			y2 = new_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double end_to_closest_new = sqrt(dx * dx + dy * dy);
			Position end_to_closest_on_new(xx, yy, 0);

			if((end_to_closest_exist <= 1 and start_to_closest_new <= 1 and end_to_closest_on_exist.getDistance(start_to_closest_on_new) >= ((new_dist + exist_dist)/2.0)/3.0) or (end_to_closest_new <= 1 and start_to_closest_exist <= 1 and end_to_closest_on_new.getDistance(start_to_closest_on_exist) >= ((new_dist + exist_dist)/2.0)/3.0) or (start_to_start_dist <= 1 and end_to_closest_exist <= 1) or (start_to_start_dist <= 1 and end_to_closest_new <= 1) or (start_to_closest_exist <= 1 and end_to_closest_exist <= 1) or (start_to_closest_new <= 1 and end_to_closest_new <= 1) or (end_to_end_dist <= 1 and start_to_closest_exist <= 1) or (end_to_end_dist <= 1 and start_to_closest_new <= 1)){
				close_to_existing = true;
				break;
			}
		}
		if(close_to_existing == true){
			return true;
		}
		for(int i = 0; i < highway_stack_completed.size(); i++){
			Position new_point_start = new_point.point;
			Position new_point_end;
			Position exist_point_start;
			Position exist_point_end;
			if(new_point.direction == true){
				new_point_end = new_point.right_point;
				exist_point_start = highway_stack_completed[i].point;
				exist_point_end = highway_stack_completed[i].right_point;
			}
			else{
				new_point_end = new_point.left_point;
				exist_point_start = highway_stack_completed[i].point;
				exist_point_end = highway_stack_completed[i].left_point;
			}
			double new_dist = new_point_start.getDistance(new_point_end);
			double exist_dist = exist_point_start.getDistance(exist_point_end);
			double start_to_start_dist = new_point_start.getDistance(exist_point_start);
			double end_to_end_dist = new_point_end.getDistance(exist_point_end);
			double start_to_end_dist = new_point_start.getDistance(exist_point_end);
			double end_to_start_dist = new_point_end.getDistance(exist_point_start);
			if((start_to_start_dist <= 1 and end_to_end_dist <= 1) or (start_to_end_dist <= 1 and end_to_start_dist <= 1) or (start_to_start_dist <= 1 and start_to_end_dist <= 1) or (end_to_end_dist <= 1 and end_to_start_dist <= 1)){
				close_to_completed = true;
				break;
			}
			double x = new_point_start.getX();
			double y = new_point_start.getY();
			double x1 = exist_point_start.getX();
			double y1 = exist_point_start.getY();
			double x2 = exist_point_end.getX();
			double y2 = exist_point_end.getY();
			double A = x - x1;
			double B = y - y1;
			double C = x2 - x1;
			double D = y2 - y1;
			double dot = A * C + B * D;
			double len_sq = C * C + D * D;
			double param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			double xx, yy;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			double dx = x - xx;
			double dy = y - yy;
			double start_to_closest_exist = sqrt(dx * dx + dy * dy);
			Position start_to_closest_on_exist(xx, yy, 0);
			x = new_point_end.getX();
			y = new_point_end.getY();
			x1 = exist_point_start.getX();
			y1 = exist_point_start.getY();
			x2 = exist_point_end.getX();
			y2 = exist_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double end_to_closest_exist = sqrt(dx * dx + dy * dy);
			Position end_to_closest_on_exist(xx, yy, 0);
			x = exist_point_start.getX();
			y = exist_point_start.getY();
			x1 = new_point_start.getX();
			y1 = new_point_start.getY();
			x2 = new_point_end.getX();
			y2 = new_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double start_to_closest_new = sqrt(dx * dx + dy * dy);
			Position start_to_closest_on_new(xx, yy, 0);
			x = exist_point_end.getX();
			y = exist_point_end.getY();
			x1 = new_point_start.getX();
			y1 = new_point_start.getY();
			x2 = new_point_end.getX();
			y2 = new_point_end.getY();
			A = x - x1;
			B = y - y1;
			C = x2 - x1;
			D = y2 - y1;
			dot = A * C + B * D;
			len_sq = C * C + D * D;
			param = -1;
			if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;
			if (param < 0) {
				xx = x1;
				yy = y1;
			}
			else if (param > 1) {
				xx = x2;
				yy = y2;
			}
			else {
				xx = x1 + param * C;
				yy = y1 + param * D;
			}
			dx = x - xx;
			dy = y - yy;
			double end_to_closest_new = sqrt(dx * dx + dy * dy);
			Position end_to_closest_on_new(xx, yy, 0);

			if((end_to_closest_exist <= 1 and start_to_closest_new <= 1 and end_to_closest_on_exist.getDistance(start_to_closest_on_new) >= ((new_dist + exist_dist)/2.0)/3.0) or (end_to_closest_new <= 1 and start_to_closest_exist <= 1 and end_to_closest_on_new.getDistance(start_to_closest_on_exist) >= ((new_dist + exist_dist)/2.0)/3.0) or (start_to_start_dist <= 1 and end_to_closest_exist <= 1) or (start_to_start_dist <= 1 and end_to_closest_new <= 1) or (start_to_closest_exist <= 1 and end_to_closest_exist <= 1) or (start_to_closest_new <= 1 and end_to_closest_new <= 1) or (end_to_end_dist <= 1 and start_to_closest_exist <= 1) or (end_to_end_dist <= 1 and start_to_closest_new <= 1)){
				close_to_existing = true;
				break;
			}
		}
		// cout << "start_highway " << start_highway << " middle_highway " << middle_highway << " end_highway " << end_highway << " close_to_existing " << close_to_existing << endl;
		if(close_to_completed == true){
			return true;
		}

		if(start_highway >= 0 and middle_highway >= 0 and end_highway >= 0){
			return true;
		}
		else if(start_highway == -1 and (middle_highway == -1 or end_highway == -1)){
			return false;
		}
		else if(start_highway >= 0 and middle_highway == -1 and end_highway == -1){
			return false;
		}
		else{
			return true;
		}
	}

private:
	int length;
	int height;
	double distance_threshold;
	double move[300];  
	double rotate[300];
	int numMoves, numRotates;
	int top_point_decisions;
	int decision_limit;
	vector< vector<int> > highway_grid;
	vector< vector< vector< pair<int, int> > > > highway_grid_connections;
	vector<Highway> highways;
	// priority_queue<DecisionPoint> highway_queue;
	vector<DecisionPoint> highway_stack;
	vector<DecisionPoint> highway_stack_completed;
	DecisionPoint last_position;
	int last_highway;
	Position current_target;
	Position middle_of_highway;
	DecisionPoint top_point;
	vector< vector<int> > traveled_grid;
	vector< vector<double> > path_to_top_point;
	bool highways_complete;
	bool go_to_top_point;
	bool go_to_farthest_on_grid;
	bool too_close;
	bool too_close_front;
	double dist_travelled_so_far;
	double avg_left;
	double avg_right;
	double avg_count;
};

#endif