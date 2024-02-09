/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include "m1.h"
#include "m2.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include "globals.h"


/**************************Global Variables********************************/


//creates a map of Streets that start with each letter with alphabetical key 
std::unordered_map<std::string, std::vector<Street>> streets_by_name;
// a map to access the streets by the street id 
std::unordered_map<StreetIdx, Street> streets;
// 2D vector that stores the street segments that connect to an intersection
std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;
// a map to access OSMNodes by their OSMid
std::unordered_map< OSMID, const OSMNode*> OSMid_Nodes;
std::unordered_map<OSMID, const OSMWay*> OSMid_Ways;
// maps that holds the street ids by letter
std::unordered_map <char, std::vector<StreetIdx>> ID_by_letter;
// vector that stores the lengths of each street segment
std::vector<double> segment_lengths;
// vectors that stores that speed limit for each street segment 
std::vector<double> segment_speedLimits;

const std::vector<char> alphanumeric_values{'0', '1', '2', '3', '4', '5', '6', '7',
                                             '8', '9', 'a', 'b', 'c',  'd', 'e', 'f', 
                                             'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
                                             'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                              'w', 'x', 'y', 'z'};
const int num_alphanumeric = 36;

double avg_lat;

/**************************************************************************/

/****************************Helper Functions Declaration******************************/

// stores street segments that connect at each intersectipon in 2D array intersection_street_segments
void loadIntersectionStreetSegments();
// function loads all the streets
void getStreetsLoaded();
// function loads all segments and intersections associated with a street
void getStreetSegmentsAndIntersections();


// loads the map of OSMNodes by OSMid's 
void loadOSMNodesByIdNumber();
// helper function for TravelTime that reuturns the results of distance/speed for a street segment
double getTravelTime(StreetSegmentIdx seg_id);
// computes and stores the average latitude for a city
void AvgLat();



/***************************************************************************/


// Loads a map streets.bin and the corresponding osm.bin file
// Can further process and organize the data if you wish.
// Returns true if successful, false if an error prevents map loading.
// Speed Requirement --> moderate
bool loadMap(std::string map_path) {

    if (!loadStreetsDatabaseBIN(map_path)){
        return false;
    }

    std::string osm_map_path = map_path;
    osm_map_path.replace(osm_map_path.find("street"), sizeof("street"), "osm");
    if(!loadOSMDatabaseBIN(osm_map_path)){
        return false;
    }
    
    // stores the data for street segments connecting to intersections
    loadIntersectionStreetSegments();
    // gets the streets loaded into an unordered map
    getStreetsLoaded();
    // gets the data loaded into the structs Streets that are in the unordered_map
    getStreetSegmentsAndIntersections();
    // gets the OSM node data loaded into the map
    loadOSMNodesByIdNumber();
    // stores the average latitude for the city 
    AvgLat();

    return true;

}

// Close the map (if loaded)
// Speed Requirement --> moderate
void closeMap() {
    // Closes the database and clears all the data structures
    closeStreetDatabase();
    closeOSMDatabase();
    intersection_street_segments.clear();
    segment_lengths.clear();
    segment_speedLimits.clear();
    streets_by_name.clear();
    OSMid_Nodes.clear();
    ID_by_letter.clear();
    street_points.clear();
    streets.clear();
    OSMid_Nodes.clear();
    OSMid_Ways.clear();
    clearDatabases();
    std::cout << "Map closed" << std::endl;
}

// Function finds the distance between two points
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    // retreive the latitude and lonitude from the two points 
    // convert them from degree to radians 
    double Lat1 = point_1.latitude()*kDegreeToRadian;
    double Lat2 = point_2.latitude()*kDegreeToRadian;
    double Lon1 = point_1.longitude()*kDegreeToRadian;
    double Lon2 = point_2.longitude()*kDegreeToRadian;
    // calculate the latitude average 
    double LatAvg = (Lat1 + Lat2)/2; 
    // compute the x and y coordinates of the two points
    double x1 = (kEarthRadiusInMeters * Lon1 * cos(LatAvg));
    double x2 = (kEarthRadiusInMeters * Lon2 * cos(LatAvg));
    double y1 = (kEarthRadiusInMeters * Lat1);
    double y2 = (kEarthRadiusInMeters * Lat2);
    // compute the distance between two points given their x and y coordinates
    double distance = sqrt(pow(y2-y1, 2) + pow(x2-x1, 2));
    // return the value of the distance between the two points
    return distance;
}

// Function finds the segment length given a segment id
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    int number_curve_points = getStreetSegmentInfo(street_segment_id).numCurvePoints;
    // get the begginning and end points of the street segment
    IntersectionIdx from = getStreetSegmentInfo(street_segment_id).from;
    IntersectionIdx to = getStreetSegmentInfo(street_segment_id).to;
    double total_distance = 0;
    // check to see if the segment has curve points 
    if(number_curve_points > 0){
        // retreive the first and last curve points of the segment
        LatLon first_curve_point = getStreetSegmentCurvePoint(street_segment_id, 0);
        LatLon last_curve_point = getStreetSegmentCurvePoint(street_segment_id, number_curve_points-1);
        // check if segment has more thamn one curve point
        if(number_curve_points > 1){
            // loop through all curve points and sum the distance between them
            for(int i = 0; i < number_curve_points-1; i++){
                // set lead one curve point ahead of tail
                LatLon lead = getStreetSegmentCurvePoint(street_segment_id, i +1); 
                LatLon trail = getStreetSegmentCurvePoint(street_segment_id, i);
                // add indivudual curve point distances to the total
                total_distance += findDistanceBetweenTwoPoints(lead, trail); 
            }
        }
        // sum distance between start of segment and first curve point
        total_distance += findDistanceBetweenTwoPoints(getIntersectionPosition(from),first_curve_point);
        // sum distance between last curve point and end of segment 
        total_distance += findDistanceBetweenTwoPoints(last_curve_point, getIntersectionPosition(to));
    }
    // if segment is a straight line compute distance between intersections that bound it 
    else{
        total_distance = findDistanceBetweenTwoPoints(getIntersectionPosition(from), getIntersectionPosition(to));
    }
    // return the sum of the distance between the start and end of the segment 
    return total_distance;
}

// Returns the travel time to drive from one end of a street segment 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    double travel_time = getTravelTime(street_segment_id);
    return travel_time;
}
// Helper function calculates the travel time by (t = distance/speed) given the segment id
double getTravelTime(StreetSegmentIdx seg_id){
    return ( (segment_lengths[seg_id])/(segment_speedLimits[seg_id]));
}
// Returns all intersections reachable by traveling down one street segment 
// from the given intersection (hint: you can't travel the wrong way on a 
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Corner case: cul-de-sacs can connect an intersection to itself 
// (from and to intersection on  street segment are the same). In that case
// include the intersection in the returned vector (no special handling needed).
// Speed Requirement --> high //2D vector that stores the 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    //declare vector for function output
    std::vector<IntersectionIdx> adjacent_intersections;
    std::vector<StreetSegmentIdx> connected_SS = findStreetSegmentsOfIntersection(intersection_id);
    //for loop thru SS around intersection (global variable)
    for (int i = 0 ; i < getNumIntersectionStreetSegment(intersection_id); i++){
        //getStreetSegmentInfo(SSID) get to and from intersections (.from,(),  .to())
        IntersectionIdx from = getStreetSegmentInfo(connected_SS[i]).from;
        IntersectionIdx to = getStreetSegmentInfo(connected_SS[i]).to;
        //if one way, only append "to"
        if (to == from){
            adjacent_intersections.push_back(to);
            i++;
        } else if (getStreetSegmentInfo(connected_SS[i]).oneWay){
            if (from == intersection_id){
                adjacent_intersections.push_back(to);
            }
        } else { //if not one way, append other intersection
            if (from == intersection_id){
                adjacent_intersections.push_back(to);
            } else {
                adjacent_intersections.push_back(from);
            }
        }
    }
    return (getUniqueVectors(adjacent_intersections));
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to 
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){ // Richelle 12:50am - function is complete, passes all test cases
    // get the size of total intersections in the city
    IntersectionIdx total_intersections = intersection_street_segments.size();
    // initilize the index for the closest intersection and smallest distance
    IntersectionIdx closest_intersection_idx = 0;
    // Sets the  distance between the very first intersection and my_position as the minimum distance currently found.
    double smallest_distance = findDistanceBetweenTwoPoints(my_position, getIntersectionPosition(0));
    double current_distance = 0;
    
    // iterate through each intersection position
    for (int i = 0; i < total_intersections; ++i){
        // get the intersection location
        // get the distance between my_position and the location of the intersection
        current_distance = findDistanceBetweenTwoPoints(my_position, getIntersectionPosition(i));
        // check if the distance is smaller than the smallest distance stored
        if (current_distance < smallest_distance){
            // if true, change the smallest distance and closest_intersection_idx
            smallest_distance = current_distance;
            closest_intersection_idx = i;
        }
    }
    // return the index of the closest intersection
    // IntersectionIdx closest_intersection_idx = 0;
    return closest_intersection_idx;

}

// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    // output the street segment stored in the 2D vector created in loadIntersectionStreetSegment()
    return intersection_street_segments[intersection_id];
}


// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    //find the street from the map
    std::unordered_map<int, Street>::iterator street = streets.find(street_id);
    // get the intersections of that street
    std::vector<IntersectionIdx> intersections_IDX = getUniqueVectors((street->second).street_intersections);
    return intersections_IDX;
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2){
    // get the two street's data and store them
    std::unordered_map<int, Street>::iterator street1 = streets.find(street_id1);
    std::unordered_map<int, Street>::iterator street2 = streets.find(street_id2);
    std::vector<IntersectionIdx> common_intersections;
    // Declare a data structure to store the return data
    std::vector<IntersectionIdx> inter1ID((street1->second).street_intersections.begin(), (street1->second).street_intersections.end());
    std::vector<IntersectionIdx> inter2ID((street2->second).street_intersections.begin(), (street2->second).street_intersections.end());
    
    for (IntersectionIdx const& intersection : inter1ID){
        // find to check if there are any common intersections between the two streets
        if (std::find(inter2ID.begin(), inter2ID.end(), intersection) != inter2ID.end()){
            // append the id to the vector when found
            common_intersections.push_back(intersection);
        }
    }
    return (getUniqueVectors(common_intersections));
}


// Returns all street ids corresponding to street names that start with the 
// given prefix 
// The function should be case-insensitstd::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id);ive to the street prefix. 
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to 
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty 
// (length 0) vector. 
// You can choose what to return if the street prefix passed in is an empty 
// (length 0) string, but your program must not crash if street_prefix is a 
// length 0 string.
// Speed Requirement --> high 
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    // declare and initialize the following data structures and variables
    std::vector<StreetIdx> street_ids;
    street_prefix = noSpacesAndLowercase(street_prefix);
    std::string curr_street;
    double prefix_length = street_prefix.length();
    

    if (prefix_length > 2){
        // get the first 3 strings of the prefix
        std::string first_3 = street_prefix.substr(0,3);

        // iterate through the names and check if the prefix matches any street names
        for (int i = 0; i < streets_by_name[first_3].size(); i++){
            curr_street = streets_by_name[first_3][i].street_name;
            //curr_street = noSpacesAndLowercase(curr_street);
            curr_street.resize(prefix_length);
            if (street_prefix == curr_street){
                // if true, then store the street id into the vector
                street_ids.push_back(streets_by_name[first_3][i].street_id);
            }
        }
    // check if length of the prefix is 2
    } else if (prefix_length == 2){
        // check for the street in the respective look-up-map
        for (char j = 0; j <num_alphanumeric; j++){
            std::string first_3 = street_prefix + alphanumeric_values[j];
            for (int i = 0; i < streets_by_name[first_3].size(); i++){
                // when found, store the street id
                street_ids.push_back(streets_by_name[first_3][i].street_id);
            }
        }
    } else {
        street_ids = ID_by_letter[street_prefix[0]];
    }
    
    // return the unique list of the street ids
    return getUniqueVectors(street_ids);
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id){
    double street_length = 0.0;
    // get the street from the unordered_map
    std::unordered_map<StreetSegmentIdx, Street>::iterator street = streets.find(street_id);
    // find the street segments of the given street and store them
    std::vector<StreetSegmentIdx> segments_id = (street->second).street_segments;
    // iterate through each segment_id and add the found length of the segment
    for (StreetSegmentIdx const& seg_id: segments_id){
        street_length += findStreetSegmentLength(seg_id);
    }
    return street_length;
}

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POItype){ 
    // declare and initialize the following
    int num_of_POIs = getNumPointsOfInterest();
    POIIdx curr_closest_POI = 0;
    double curr_closest_distance = 0;
    bool initial = true;

    // iterate through the points of interest
    for (int i = 0; i < num_of_POIs; i++){
        if (POItype == getPOIType(i) && initial){
            // if the POI is found, then get the distance between the location and the POI
            curr_closest_distance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(i));
            curr_closest_POI = i;
            initial = false;
        }
        else if (POItype == getPOIType(i)) {
            if (findDistanceBetweenTwoPoints(my_position, getPOIPosition(i)) < curr_closest_distance){
                curr_closest_distance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(i));
                curr_closest_POI = i;
            }
        }
    }
    return curr_closest_POI;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    
    // Declare/initialize the following structures
    int num_of_points = getNumFeaturePoints(feature_id);
    double Lat1, Lat2, Lon1, Lon2, LatAvg, x1, x2, y1, y2, trapezoid_area;
    LatLon trail;
    double area = 0;

    // Get the feature points of the following
    LatLon lead = getFeaturePoint(feature_id, 0);
    if ((lead.latitude() != getFeaturePoint(feature_id, num_of_points-1).latitude()) && (lead.longitude() != getFeaturePoint(feature_id, num_of_points-1).longitude())){
        return 0;
    }
    // Calculate the area of the points
    for (int i = 0; i < num_of_points-1; i++){
        trail = lead;
        lead = getFeaturePoint(feature_id, i+1);
        // retreive the latitude and lonitude from the two points 
        // convert them from degree to radians 
        Lat1 = lead.latitude()*kDegreeToRadian;
        Lat2 = trail.latitude()*kDegreeToRadian;
        Lon1 = lead.longitude()*kDegreeToRadian;
        Lon2 = trail.longitude()*kDegreeToRadian;
        // Calculate latitude average
        LatAvg = (Lat1 + Lat2)/2; 
        // compute the x and y coordinates of the two points
        x1 = (kEarthRadiusInMeters * Lon1 * cos(LatAvg));
        x2 = (kEarthRadiusInMeters * Lon2 * cos(LatAvg));
        y1 = (kEarthRadiusInMeters * Lat1);
        y2 = (kEarthRadiusInMeters * Lat2);
        // Find and add the area
        trapezoid_area = (y2-y1)*(x1+x2)/2;
        area += trapezoid_area;

    }
    if (area < 0){
        return (area*-1);
    }
    return area;
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is 
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    std::unordered_map<OSMID, const OSMNode *>::iterator node_entity;
    node_entity = OSMid_Nodes.find(OSMid);
    // get the number of tags attributed to the node
    int num_tags = getTagCount(node_entity->second);
    // check for the key that matches with the input key
    for(int i = 0; i < num_tags; ++i){
 		std::string key_match,value;
         // matches the value with it's corresponding key for that node
 		std::tie(key_match,value) = getTagPair(node_entity->second,i);
        if(key_match == key){
            // output the value of the nodes key 
            return value;
        }
    }
    std::string empty_str = " ";
    return empty_str;
}


/***************************************************************************************************/
/**************************************Helper Functions**********************************************/
/***************************************************************************************************/

// Function loads the OSM nodes and ways to the data structure
void loadOSMNodesByIdNumber(){
    int num_nodes = getNumberOfNodes();
    // loop through the number of nodes in the city 
    for(int i = 0; i < num_nodes; i++){
         // get the pointer to the node at the ith index 
        const OSMNode *node = getNodeByIndex(i); 
         // get the OSMid of the node at the ith index
        OSMID node_id = node->id();
         // insert the node_id and node into the hash table
         // will be able to access the node ptr by searching the OSMid values in the hash table 
        OSMid_Nodes[node_id] = node;
    }
    
    int num_ways = getNumberOfWays();
    // Loop through the number of ways in the city
    for(int j = 0; j < num_ways; j++){
        // get the pointer to the way at ith index
        const OSMWay *way = getWayByIndex(j);
        // get the OSMid of the node at the ith index
        OSMID way_id = way->id();
        // Populate the OSMid_ways function by setting the way_id as the key 
        // and way OSMWay as its value
        OSMid_Ways[way_id] = way;
    }
}

// function that stores the sorted data for intersection street segments into a 2d vector 
void loadIntersectionStreetSegments(){
    int numIntersections = getNumIntersections();
    // index of the first layer corresponds the the intersection id
    intersection_street_segments.resize(numIntersections);
    // iterate through all itersections and populate it with a vector that contains street segments 
    for(int intersection = 0; intersection < numIntersections; ++intersection){
        int numSegAtIntersection = getNumIntersectionStreetSegment(intersection);
        for(int i = 0; i < numSegAtIntersection; ++i){
            StreetSegmentIdx street_segment_id = getIntersectionStreetSegment(intersection, i);
            // save segment connected to intersection in the second layer of the 2D vector
            intersection_street_segments[intersection].push_back(street_segment_id);
        } 
    }
}

// Function returns a string that is all lowercase with no spaces
std::string noSpacesAndLowercase(std::string String){
    transform(String.begin(), String.end(), String.begin(), ::tolower);
    String.erase(std::remove_if(String.begin(), String.end(), ::isspace), String.end());
    return String;
}

// Loads all the street data into the structs of the unordered map
void getStreetsLoaded(){
     // iterate through streets. Load street data onto the struct
    for (int i = 0 ; i<num_alphanumeric; i++){
        ID_by_letter[alphanumeric_values[i]];
        for(int j = 0; j<num_alphanumeric; j++){
            for(char k = 0; k<num_alphanumeric; k++){
                streets_by_name[std::string(1, alphanumeric_values[i]) + alphanumeric_values[j] + alphanumeric_values[k]];
            }
        }
    }

    for (StreetIdx i = 0; i < getNumStreets(); i++){
        // create a struct
        streets[i] = Street();
        streets[i].street_id = i;
        streets[i].street_name = noSpacesAndLowercase(getStreetName(i));
        streets[i].street_name_caps = getStreetName(i);
        // Populate the respective data structures with the street names and data 
        ID_by_letter[streets[i].street_name[0]].push_back(i);
        std::string first_3 = streets[i].street_name.substr(0,3);
        std::string first_3_lower = noSpacesAndLowercase(first_3);
        streets_by_name[first_3_lower].push_back(streets[i]);
    }

}

// Function loads the street segments into each street by comparing street id and segment_street id
void getStreetSegmentsAndIntersections(){
    // Iterate through the segments
    std::unordered_map<int, Street>::iterator street;
    StreetSegmentInfo segment;
    for (StreetSegmentIdx seg_idx = 0; seg_idx < getNumStreetSegments(); seg_idx++){
        segment = getStreetSegmentInfo(seg_idx);
        segment_lengths.push_back(findStreetSegmentLength(seg_idx));
        segment_speedLimits.push_back(segment.speedLimit);
        // Find the street id if exists in our unordered_map structure "streets"
        street = streets.find(segment.streetID);
        // Compare the street_ids together
        if(segment.streetID == (street->second).street_id){
            // add it to the struct's street_segments vector
            (street->second).street_segments.push_back(seg_idx);
            (street->second).street_intersections.push_back(segment.from);
            (street->second).street_intersections.push_back(segment.to);
        }
    }

}

std::vector<int> getUniqueVectors(std::vector<int> vector_data){
    // declare an iterator
    std::vector<IntersectionIdx>::iterator it;
    // Sort the vector to find the dublicates
    std::sort(vector_data.begin(), vector_data.end());
    // Remove all dublicates
    it = std::unique(vector_data.begin(), vector_data.end());
    // put the unique elements vector in the original vector
    vector_data.resize( std::distance(vector_data.begin(),it));
    // Retrieve the vector data with no dublicate intersections
    return vector_data;
}

void AvgLat(){

    int num_intersections = getNumIntersections();
    int sum_lat = 0;
    // loops through all the intersections and finds their latitude value
    for(int inter_id = 0; inter_id < num_intersections; inter_id++){
        LatLon inter_pos = getIntersectionPosition(inter_id);
        double inter_lat = inter_pos.latitude();
        // sums the total of each intersection latitude
        sum_lat += inter_lat;
    }
    // computes the average latitude
    avg_lat = sum_lat/num_intersections;

}
