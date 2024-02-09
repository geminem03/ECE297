
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include <gtk/gtk.h>
#include <gtk/gtkcomboboxtext.h>
#include "color.hpp"
#define NO_EDGE -1

// returns the path with the smallest travel time
bool dijkstra(int startID, int destID, std::vector<StreetSegmentIdx>& optimalPath, double turn_penalty);


struct IntersectionNode {
    int intersectionID;
    double shortestTime;
    IntersectionNode(int id, double time) : intersectionID(id), shortestTime(time) {}
    bool operator<(const IntersectionNode& other) const {
        return shortestTime > other.shortestTime;
    }
};

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const std::vector<StreetSegmentIdx>&  path, const double turn_penalty){ /* Richelle @ 5:43pm*/
    double path_travel_time = 0;
    int size = path.size();
    // iterate through each segment
    for (int i = 0; i < size; i++){
        StreetSegmentInfo segment = getStreetSegmentInfo(path[i]);
        // add the path travel time
        path_travel_time += findStreetSegmentTravelTime(path[i]);
        if(i < (size-1)){
            // check if the next segment's street id is equal to the current segment's street id
            if(segment.streetID != getStreetSegmentInfo(path[i+1]).streetID){
                // if false then add turn penalty
                path_travel_time += turn_penalty;
            }
        } 
    }
    return path_travel_time;
}


// Returns a path (route) between the start intersection (intersect_id.first)
// and the destination intersection (intersect_id.second), if one exists. 
// This routine should return the shortest path
// between the given intersections_, where the time penalty to turn right or
// left is given by turn_penalty (in seconds).  If no path exists, this routine
// returns an empty (size == 0) vector.  If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids, const double turn_penalty) 
{
    // initialize path and visited vectors
    std::vector<StreetSegmentIdx> path;
    std::vector<bool> visited(getNumIntersections(), false);
    //bool path_found = breadthFirst(intersect_ids.first, intersect_ids.second, path);
    bool path_found = dijkstra(intersect_ids.first, intersect_ids.second, path, turn_penalty);

    // If path is found, display info in the terminal
    if(path_found) {
        // std::cout << "A path has been found between " << getIntersectionName(intersect_ids.first) << "( " << intersect_ids.first << " )";
        // std::cout << " and " << getIntersectionName(intersect_ids.second) << "( " << intersect_ids.second << " )" << std::endl;
        // std::cout << "Travel time: " << computePathTravelTime(path, turn_penalty) << std::endl;
        std::reverse(path.begin(), path.end());
        return path;
    } else {
        // std::cout << "NO path has been found between " << getIntersectionName(intersect_ids.first) << "( " << intersect_ids.first << " )";
        // std::cout << " and " << getIntersectionName(intersect_ids.second) << "( " << intersect_ids.second << " )" << std::endl;

        // return an empty path vector
        std::vector<StreetSegmentIdx> no_path;;
        return no_path;
    }
}

bool dijkstra(int startID, int destID, std::vector<StreetSegmentIdx>& optimalPath, double turn_penalty) {
    // Create a priority queue to hold nodes to visit
    std::priority_queue<IntersectionNode> toVisit;
    // Create a vector to keep track of visited nodes
    std::vector<bool> visited(getNumIntersections(), false);
    // Create a vector to store the shortest time to reach each intersection visited
    std::vector<double> shortestTime(getNumIntersections(), std::numeric_limits<double>::infinity());
    // Store all the distance from the current to the end intersection

    // Create a vector to store the previous intersection of each intersection visited
    std::vector<StreetSegmentIdx> prevIntersection(getNumIntersections(), NO_EDGE);
    // Create a vector to store the previous street of each intersection visited
    std::vector<StreetSegmentIdx> prevStreet(getNumIntersections(), NO_EDGE);

    // Add the starting node to the queue with a travel time of 0 and mark it as visited
    toVisit.push(IntersectionNode(startID, 0.0));
    shortestTime[startID] = 0.0;

    // while the queue is not empty
    while (!toVisit.empty()) {
        // Get the next node to visit from the front of the queue
        IntersectionNode currNode = toVisit.top();
        toVisit.pop();

        // if already visited, skip
        if(visited[currNode.intersectionID]){
            continue;
        }
        visited[currNode.intersectionID] = true;

        // Check if this is the destination node stop search
        if (currNode.intersectionID == destID) {
            // Traverse the prevIntersection vector to build the optimal path of street segments
            StreetSegmentIdx currSeg = prevIntersection[currNode.intersectionID];
            while (currSeg != NO_EDGE) {
                optimalPath.push_back(currSeg);
                StreetSegmentInfo currSegInfo = getStreetSegmentInfo(currSeg);
                int currSegFrom = currSegInfo.from;
                currSeg = (currSegFrom == currNode.intersectionID) ? prevIntersection[currSegInfo.to] : prevIntersection[currSegFrom];
                currNode.intersectionID = (currSegFrom == currNode.intersectionID) ? currSegInfo.to : currSegFrom;
            }
            return true;
        }

        // Get the outgoing edges for the current node
        std::vector<StreetSegmentIdx> outgoingEdges = findStreetSegmentsOfIntersection(currNode.intersectionID);

        // Loop through all the outgoing edges
        for (int edge_idx = 0; edge_idx < outgoingEdges.size(); edge_idx++) {
            // Get the current edge and its information
            StreetSegmentIdx edge = outgoingEdges[edge_idx];
            StreetSegmentInfo edgeInfo = getStreetSegmentInfo(edge);

            // If this is a one-way street and we are not going in the right direction, skip this edge
            if (edgeInfo.oneWay && edgeInfo.from != currNode.intersectionID) {
                continue;
            }

            // Best time <- total time
            // segment travel time <- travel time
            // Get the ID of the next intersection on the edge
            int nextID;
            if (edgeInfo.from == currNode.intersectionID){
                nextID = edgeInfo.to;
            } else {
                nextID = edgeInfo.from;
            }

            // Calculate the travel time to reach the next intersection
            double segmentTravelTime = findStreetSegmentTravelTime(edge);
            // Calculate the total time to reach the next intersection via the current edge
            double totalTime = currNode.shortestTime + segmentTravelTime;

            // Add a turn penalty if the current street is different from the previous street
            // Check if the current street is different from the previous street
            if (prevIntersection[currNode.intersectionID] != NO_EDGE) {
                StreetSegmentInfo prevEdgeInfo = getStreetSegmentInfo(prevIntersection[currNode.intersectionID]);
                if (prevEdgeInfo.streetID != edgeInfo.streetID) {
                    totalTime += turn_penalty;
                }
            }

            // Check if the total time to reach the next intersection is shorter than the current shortest time
            if (totalTime < shortestTime[nextID]){
                // Update the shortest time to reach the next intersection
                shortestTime[nextID] = totalTime;
                // Update the previous intersection and street
                prevIntersection[nextID] = edge;
                // Add the next node to the queue
                toVisit.push(IntersectionNode(nextID, totalTime));
            }
        }
    }
    // If we reach this point, there is no path from the start node to the destination node
    return false;
}