#pragma once

// Loads data for intersection latitude/longitude and Cartesian values
void loadIntersectionData();
// Loads the XY positions of the from and to intersections for each street segment
void loadStreetSegmentData();
// Loads the latitude/longitude positions of all the map features
void loadFeatureData();
// Loads the names and attributes of points of interest (POIs)
void loadPOIData();
// Loads the OpenStreetMap (OSM) tag values for streets
void loadOSMHighwayTags();
// Loads all the relations of the listed attributes from OSM
void loadOSMRelations();
// Loads the Cartesian coordinates of all the street segments
void loadStreetCoordinates();
// Loads all the street points into the data structure
void loadStreetPoints();
// Loads all the osm values
void loadSubwayOSMValues();
// initialized latlon min and max to false values before check
void initializeMaxMin();
// sets the min an max latlon based on each intersection check
void setMaxMin(int inter_id);
// helper function that sets intersection data to the struct for each intersection
void setIntersectionData(int inter_id, Intersection_data& inter_data);
// stores the main information for each street segment
void loadStreetSegmentDataHelper(int& ss_id, StreetSegmentInfo& street_seg, std::unordered_map<int, Street>::iterator& street);
// stores the curve point postions in a vector
void loadCurvePoints(int& ss_id);
// loads the osm highway tags into the struct 
void loadHighwayOSMTags(int& ss_id, StreetSegmentInfo& street_seg);
