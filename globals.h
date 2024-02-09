#ifndef GLOBALS_H
#define GLOBALS_H

#include <chrono>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"

/*************************************************************************/
/****************************Global Structs*******************************/
/*************************************************************************/


// a struct that contais that key atributes that need to be accessed 
struct Street {
   StreetIdx street_id;
   std::string street_name;
   std::string street_name_caps;
   std::vector<IntersectionIdx> street_intersections;
   std::vector<StreetSegmentIdx> street_segments;
   
   ~Street(){
      street_intersections.clear();
      street_segments.clear();
   }
};


//To help change between maps easier (also changes languages for fonts)
struct map_info {
   std::string map_name;
   std::string file_name;
   std::string font_name;
};

// structure that holds the spatial data for intersections
struct Intersection_data{
   // m3 updates
   IntersectionIdx inter_id;
   std::vector<IntersectionIdx> adjacent_intersections;
   std::vector<StreetSegmentIdx> connected_street_segs; 
   // m2 atributes  
   LatLon position;
   ezgl::point2d xy_loc;
   std::string inter_name;
   bool highlight = false;

   ~Intersection_data(){
      adjacent_intersections.clear();
      connected_street_segs.clear();
   }

};

// contains spatial and street data of street segments
struct StreetSegment_Data{
   // m3 updates
   StreetSegmentIdx ss_id; 
   StreetIdx street_id;
   std::string street_name;
   IntersectionIdx from_id;          
   IntersectionIdx to_id;  
   bool one_way = false;
   double travel_time;  
   bool highlight_path = false;
   // street name all lower case no space
   std::string street_name_LCNS;
   double segment_length;
   double speed_limit;
   LatLon from_pos;
   LatLon to_pos;
   ezgl::point2d from_xy;
   ezgl::point2d to_xy;
   std::vector<ezgl::point2d> curve_points;
   // checks what type of way it is
   bool highway_motorway = false;
   bool highway_trunk = false;
   bool highway_motorway_link = false;
   bool highway_trunk_link = false;
   bool highway_primary = false;
   bool highway_primary_link = false;
   bool highway_secondary = false;
   bool highway_tertiary = false;
   bool highway_pedestrian = false;
   bool highway_livingstreet = false;
   bool highway_residential = false;
   bool highway_road = false;
   bool highway_other = false;

   ~StreetSegment_Data(){
      curve_points.clear();
   }
};

// contains Feautre attributes
struct Feature_Data{
   std::string feature_name;
   FeatureType feature_type;
   TypedOSMID feature_OSMID;
   std::vector<ezgl::point2d> feature_point_xy;
   bool is_closed_polygon = false;

   ~Feature_Data(){
      feature_point_xy.clear();
   }
};

// contains POI attributes
struct POIData{
   std::string POI_type;
   std::string POI_name;
   ezgl::point2d POI_xy;
   OSMID POI_NodeID;
};

struct SubwayLine { 
   std::string line_name; 
   std::string color; 
   const OSMRelation* subwayLine; 
   // stores all the stations of the subway line
   std::vector<const OSMNode*> subwayLineStations; 
   // stores all the ways of the specific subway line
   std::vector<const OSMWay*> subway_ways;
   ~SubwayLine(){
      subway_ways.clear();
      subwayLineStations.clear();
   }

}; 

struct FeatureArea {
   FeatureIdx id;
   double area;
};


/*************************************************************************/
/***************************Global Vectors********************************/
/*************************************************************************/

// stores the spatial data for all intersections
extern std::vector<Intersection_data> intersections;
extern std::unordered_map<IntersectionIdx, Intersection_data> intersection_map;
// vecotor stores the data pertinent to street segment drawings
extern std::vector<StreetSegment_Data> street_segments;
// stores the data for all the features on the map
extern std::vector<Feature_Data> features;
// stores the data for all POIs on the map 
extern std::vector<POIData> POIs;
// Stores all the subway line infos
extern std::vector<SubwayLine> subway_lines_info; 
// stores all the coordinates of the streets
extern std::unordered_map <StreetIdx, std::vector<ezgl::point2d>> street_points;
//stores the osmnode of the subway stations
extern std::vector<const OSMNode*> osmSubwayStations;


//creates a map of Streets that start with each letter with alphabetical key 
extern std::unordered_map<std::string, std::vector<Street>> streets_by_name;
// a map to access the streets by the street id 
extern std::unordered_map<StreetIdx, Street> streets;
// 2D vector that stores the street segments that connect to an intersection
extern std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;
// vectors that stores that speed limit for each street segment 
extern std::vector<double> segment_speedLimits;
// a map to access OSMNodes by their OSMid
extern std::unordered_map< OSMID, const OSMNode*> OSMid_Nodes;
// Stores all the way data with its OSMID
extern std::unordered_map<OSMID, const OSMWay*> OSMid_Ways;


/*************************************************************************/
/***************************Global Variables******************************/
/*************************************************************************/
extern bool night_mode; 
extern bool show_POI;
extern bool show_subways;
// holds the average latitude for cartesian latlon conversions
extern double avg_lat;
// global variables to define the canvas size to fit the city 
extern double max_lat;
extern double min_lat;
extern double max_lon;
extern double min_lon;


// RGB values are taken directly from Google Maps
const ezgl::color BLANK (0, 0, 0, 0);
const ezgl::color PRIMARY_COLOR(255, 196, 0);
const ezgl::color park_green(4, 109, 4);
const ezgl::color night_blue(0, 0, 64);
const ezgl::color beach_grey(68, 68, 100);
const ezgl::color light_green(195, 236, 178);
const ezgl::color light_blue(170, 218, 255);
const ezgl::color light_grey(213, 216, 219);
const ezgl::color night_grey(68,68,68);
const ezgl::color white(255, 255, 255);
const ezgl::color dark_green(0, 60, 0);
const ezgl::color glacier(195, 236, 178);
const ezgl::color night_glacier(70, 70, 70);
const ezgl::color background_color(235, 234, 235);
const ezgl::color red(235, 0, 0);

const double PI = 3.14159265358979323846;
const int MILLISECONDS_PER_SECOND = 1000;

// helper function for PartialStreetNames that returnns a simplified version of the name for parsing 
extern std::string noSpacesAndLowercase(std::string String);
// Returns unique set of vectors given a vector
extern std::vector<int> getUniqueVectors(std::vector<int> vector_data);

extern void clearDatabases();
// Unhighlights all the highlighted intersections


#endif