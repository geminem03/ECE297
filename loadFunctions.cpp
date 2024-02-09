
#include <chrono>
#include <string>
#include <vector>

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "globals.h"
#include "loadFunctions.h"
#include "math.h"


/********************************************************************************/
/********************************Load Functions**********************************/
/********************************************************************************/


// Loads data for intersection latitude/longitude and Cartesian values
void loadIntersectionData(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // Set the min and max LatLon to false values
   initializeMaxMin();

   // Loop through all the intersections in the city
   for (int inter_id = 0; inter_id < getNumIntersections(); inter_id++) {

      // load intersection data into struct to store information
      Intersection_data inter_data;
      setIntersectionData(inter_id, inter_data);

      // insert intersection data into the unordered map
      intersection_map[inter_id] = inter_data;

      // set max min latlon based on new intersection data
      setMaxMin(inter_id);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadIntersectionData took " << elapasedTime.count() << "seconds." <<std::endl;

}

// Loads the XY positions of the from and to intersections for each street segment
void loadStreetSegmentData(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // resize the street segments vector to accomodate all segments
   street_segments.resize(getNumStreetSegments());
   
   // create an iterator to access the streets in the map
   std::unordered_map<int, Street>::iterator street;

   // loop through all the street segments
   for(int ss_id = 0; ss_id < getNumStreetSegments(); ss_id++){
      // set the info for the current street segment
      StreetSegmentInfo street_seg = getStreetSegmentInfo(ss_id);
      loadStreetSegmentDataHelper(ss_id, street_seg, street);
      // stores the position of curve points in a vector 
      loadCurvePoints(ss_id);
      // loads all highway tags to the struct
      loadHighwayOSMTags(ss_id, street_seg); 
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadStreetSegmentData took " << elapasedTime.count() << "seconds." <<std::endl;
}

// Loads the latitude/longitude positions of all the map features
void loadFeatureData() {

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   int num_features = getNumFeatures();
   // reserve space in vector to avoid excessive memory allocation
   features.reserve(num_features);
   for(int feat_id = 0; feat_id < num_features; feat_id++) {
      // store data for feature name, type, and OSMID
      Feature_Data feature;
      feature.feature_name = getFeatureName(feat_id);
      feature.feature_type = getFeatureType(feat_id);
      feature.feature_OSMID = getFeatureOSMID(feat_id);
      // store xy data for each point of the feature
      int num_feat_points = getNumFeaturePoints(feat_id);
      // reserve space in vector to avoid excessive memory allocation
      feature.feature_point_xy.reserve(num_feat_points);
      for(int point_idx = 0; point_idx < num_feat_points; point_idx++) {
         // first must convert from latlon
         LatLon point_pos = getFeaturePoint(feat_id, point_idx);
         feature.feature_point_xy.emplace_back(
            x_from_lon(point_pos.longitude()),
            y_from_lat(point_pos.latitude())
         );
      }
      // determine if feature is a line or a closed polygon
      LatLon first_point_pos = getFeaturePoint(feat_id, 0);
      LatLon last_point_pos = getFeaturePoint(feat_id, num_feat_points - 1);
      if(first_point_pos == last_point_pos) {
         feature.is_closed_polygon = true;
      }
      features.push_back(feature);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadFeatureData took " << elapasedTime.count() << "seconds." <<std::endl;

}

// Loads the names and attributes of points of interest (POIs)
void loadPOIData(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   int num_POIs = getNumPointsOfInterest();
   POIs.resize(num_POIs);
   // loops through all the POIs
   for(int POI_id = 0; POI_id < num_POIs; POI_id++){
      // retreives and stores the type, name and OSMid of the POI
      POIs[POI_id].POI_type = getPOIType(POI_id);
      POIs[POI_id].POI_name = getPOIName(POI_id);
      POIs[POI_id].POI_NodeID = getPOIOSMNodeID(POI_id);
      // retreives and stores the xy position of the POI
      LatLon POI_pos = getPOIPosition(POI_id);
      double POI_lon = POI_pos.longitude();
      double POI_lat = POI_pos.latitude();
      POIs[POI_id].POI_xy.x = x_from_lon(POI_lon);
      POIs[POI_id].POI_xy.y = y_from_lat(POI_lat); 
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadPOIData took " << elapasedTime.count() << "seconds." <<std::endl;

}

// Loads all the street point2ds into its respective data structure
void loadStreetPoints(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   std::vector<ezgl::point2d>::iterator pointxy;
   for (StreetSegment_Data& segment : street_segments){
      street_points[segment.street_id].push_back(segment.from_xy);
      if (segment.curve_points.size() == 1){
         pointxy = segment.curve_points.begin();
         street_points[segment.street_id].push_back(*pointxy);
         
      } else {
         pointxy = segment.curve_points.begin();
         for (;pointxy != segment.curve_points.begin(); pointxy++){
            street_points[segment.street_id].push_back(*pointxy);
         }
      }
      street_points[segment.street_id].push_back(segment.to_xy);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadStreetPoints took " << elapasedTime.count() << "seconds." <<std::endl;

}

// loads all the subway related information
void loadSubwayOSMValues(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // initialize the vector
   std::vector<TypedOSMID> subway_stations;
   // iterate through all relations
   for (unsigned lineIndex = 0; lineIndex < getNumberOfRelations(); lineIndex++) {
      const OSMRelation* currLine = getRelationByIndex(lineIndex);
      // iterate through tag values of this relation
      for (unsigned memberIndex = 0; memberIndex < getTagCount(currLine); memberIndex++) {
         std::pair<std::string, std::string> tagPair = getTagPair(currLine, memberIndex);
         //std::cout << "tagPair1: " << tagPair.first << "|| tagPair2: "<< tagPair.second << std::endl;
         // get all relation members of this current subway line
         subway_stations = getRelationMembers(currLine);
         SubwayLine subway;
         // check if the tag value pair is route: subway
         if (tagPair.first == "route"  && tagPair.second == "subway"){
            // if true create a subway line object
            subway.subwayLine = currLine;
            
            // iterate through each member
            for (unsigned i = 0; i < subway_stations.size(); i++){
               // check if the member type is node
               if (subway_stations[i].type() == TypedOSMID::Node){
                  // get the node through seaching it id in the osmid_nodes data structure
                  const OSMNode *currNode = (OSMid_Nodes[subway_stations[i]]);
                  // iterate through the node's tag count
                  for (unsigned tagIndex = 0; tagIndex < getTagCount(currNode); tagIndex++) {
                     std::pair<std::string, std::string> tagPair2 = getTagPair(currNode, tagIndex);
                     // get the tag pair that is station = subway and push the station into the object nodes attribute 
                     if (tagPair2.first == "name"){
                        subway.subwayLineStations.push_back(currNode);
                        osmSubwayStations.push_back(currNode);
                        subway.line_name = tagPair2.second;
                        break;
                     }
                  }
                  // check if the member is a way
               } else if (subway_stations[i].type() == TypedOSMID::Way){
                  // store the way into the struct
                  const OSMWay *currWay = (OSMid_Ways[subway_stations[i]]);
                  subway.subway_ways.push_back(currWay);
               }
            }
            // Insert the object into the subways vector
            subway_lines_info.push_back(subway);
         }
      }
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadSubwayOSMValues took " << elapasedTime.count() << "seconds." <<std::endl;

}

/*************************************************************************/
/***********************Intersection Helpers******************************/
/*************************************************************************/

void initializeMaxMin(){
   // Set the min and max LatLon to false values
   max_lat = getIntersectionPosition(0).latitude();
   min_lat = max_lat;
   max_lon = getIntersectionPosition(0).longitude();
   min_lon = max_lon;
}

void setMaxMin(int inter_id){
   // Add latitude of current intersection to the total value
   // Check and store the max and min LatLon values 
   Intersection_data curr_inter= intersection_map[inter_id];
   double inter_lon = curr_inter.position.longitude();
   double inter_lat = curr_inter.position.latitude();

   max_lon = std::max(max_lon, inter_lon);
   min_lon = std::min(min_lon, inter_lon);
   max_lat = std::max(max_lat, inter_lat);
   min_lat = std::min(min_lat, inter_lat);
}

void setIntersectionData(int inter_id, Intersection_data& inter_data){

   inter_data.inter_id = inter_id;
   inter_data.inter_name = getIntersectionName(inter_id);
   inter_data.adjacent_intersections = findAdjacentIntersections(inter_id);
   inter_data.connected_street_segs = findStreetSegmentsOfIntersection(inter_id);

   // Store position and name of the intersection in the vector 
   inter_data.position = getIntersectionPosition(inter_id);
   // Set variables for the intersection LatLon
   double inter_lon = inter_data.position.longitude();
   double inter_lat = inter_data.position.latitude();
   // Set the x and y value for the point2d namespace
   inter_data.xy_loc.x = x_from_lon(inter_lon);
   inter_data.xy_loc.y = y_from_lat(inter_lat);
}

/*************************************************************************/
/***********************Street Segment Helpers****************************/
/*************************************************************************/

void loadStreetSegmentDataHelper(int& ss_id, StreetSegmentInfo& street_seg, std::unordered_map<int, Street>::iterator& street){

   // load and store all street segment info (m3 update)
   street_segments[ss_id].ss_id = ss_id;
   street_segments[ss_id].one_way = street_seg.oneWay;
   street_segments[ss_id].from_id = street_seg.from;
   street_segments[ss_id].to_id = street_seg.to;
   street_segments[ss_id].travel_time = findStreetSegmentTravelTime(ss_id);
   street_segments[ss_id].street_id = street_seg.streetID;
   // set variable to store street name (less messy)
   StreetIdx street_id = street_segments[ss_id].street_id;
   street_segments[ss_id].street_name = getStreetName(street_id);

   // set variables for the from and to intersections 
   IntersectionIdx from_inter = street_seg.from;
   IntersectionIdx to_inter = street_seg.to;
   // get the LatLon values of the from and to intersections
   street_segments[ss_id].from_pos = getIntersectionPosition(from_inter);
   street_segments[ss_id].to_pos = getIntersectionPosition(to_inter);
   // extract the longitude and latitude values of the from and to intersections
   double from_lon = street_segments[ss_id].from_pos.longitude();
   double from_lat = street_segments[ss_id].from_pos.latitude();
   double to_lon = street_segments[ss_id].to_pos.longitude();
   double to_lat = street_segments[ss_id].to_pos.latitude();
   // convert the longitude and latitude values to x and y coordinates 
   street_segments[ss_id].from_xy.x = x_from_lon(from_lon);
   street_segments[ss_id].from_xy.y = y_from_lat(from_lat);
   street_segments[ss_id].to_xy.x = x_from_lon(to_lon);
   street_segments[ss_id].to_xy.y = y_from_lat(to_lat);

   // find the street that the segment is a part of
   street = streets.find(street_seg.streetID);
   // check if the street ID found matches the current segment's street ID
   if(street_seg.streetID == (street->second).street_id){
      // store the streetID of the segment 
      street_segments[ss_id].street_id =street_seg.streetID;
   }  
   // store the name of the street that the segment is on
   street_segments[ss_id].street_name = getStreetName(street_seg.streetID);
   // store the length of the street segment
   street_segments[ss_id].segment_length = findStreetSegmentLength(ss_id);
   // store the speed limit of the segment 
   street_segments[ss_id].speed_limit = segment_speedLimits[ss_id];

}

void loadCurvePoints(int& ss_id){
   // Load the segment curve points into a vector 
   StreetSegmentInfo seg_info = getStreetSegmentInfo(ss_id);
   int num_curve_points = seg_info.numCurvePoints;
   // resize the vector to accomodate all the curve points
   street_segments[ss_id].curve_points.resize(num_curve_points);
   // loop through all the curve points that make up the segment
   for(int cp_id = 0; cp_id < num_curve_points; cp_id++){
      // get the LatLon values of the curve point
      LatLon curve_point_pos = getStreetSegmentCurvePoint(ss_id, cp_id);
      double curve_point_lon = curve_point_pos.longitude();
      double curve_point_lat = curve_point_pos.latitude();
      // convert the LatLon values to x and y coordinates and store them in the vector
      street_segments[ss_id].curve_points[cp_id].x = x_from_lon(curve_point_lon);
      street_segments[ss_id].curve_points[cp_id].y = y_from_lat(curve_point_lat);
   }
}

void loadHighwayOSMTags(int& ss_id, StreetSegmentInfo& street_seg){
   // get the osm way of the segment and categorize the segment into the following
   const OSMWay* current_way = OSMid_Ways[street_seg.wayOSMID];
   int tag_count = getTagCount(current_way); 
   for (int tag_num = 0; tag_num < tag_count; tag_num++){
      std::pair<std::string, std::string> tagPair = getTagPair(current_way, tag_num);
      if (tagPair.first == "highway"){
         if (tagPair.second == "motorway"){          
            street_segments[ss_id].highway_motorway = true; 
         } 
         else if (tagPair.second == "motorway_link"){
            street_segments[ss_id].highway_motorway_link = true;
         } 
         else if (tagPair.second == "trunk_link"){
            street_segments[ss_id].highway_trunk_link = true;
         } 
         else if (tagPair.second == "trunk"){
            street_segments[ss_id].highway_trunk = true;
         } 
         else if (tagPair.second == "primary"){
            street_segments[ss_id].highway_primary = true; 
         } 
         else if (tagPair.second == "primary_link"){
            street_segments[ss_id].highway_primary_link = true; 
         } 
         else if (tagPair.second == "secondary"||tagPair.second == "secondary_link"){
            street_segments[ss_id].highway_secondary = true;
         } 
         else if (tagPair.second == "tertiary"||tagPair.second == "tertiary_link"){
            street_segments[ss_id].highway_tertiary = true;
         } 
         else if (tagPair.second == "pedestrian"){
            street_segments[ss_id].highway_pedestrian = true;
         } 
         else if (tagPair.second == "living_street"){
            street_segments[ss_id].highway_livingstreet = true;
         }
         else if (tagPair.second == "residential"){
            street_segments[ss_id].highway_residential = true;
            
         } else if (tagPair.second == "road"){
            street_segments[ss_id].highway_road = true;
         } else {
            street_segments[ss_id].highway_other = true;
         }
         
      }
   }
}