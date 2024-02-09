#pragma once

// Convert latitude to y-coordinate on a 2D map
double y_from_lat(double lat);
// Convert longitude to x-coordinate on a 2D map
double x_from_lon(double lon);
// Convert x-coordinate on a 2D map to longitude
double lon_from_x(double x);
// Convert y-coordinate on a 2D map to latitude
double lat_from_y(double y);

// computes the centroid of a feature polygon
// copied from stackoverflow example with a few adjustments to the input parameters
ezgl::point2d compute2DPolygonCentroid(std::vector<ezgl::point2d> &vertices, int vertexCount);

//define zoom  levels of the screen
void zoom_levels(ezgl::renderer *g, int& level);
//Checks if the given number is in the given range
bool inGivenRange(int low, int high, int num);
//Returns the angle between two lines
double angleBetween2Lines(StreetSegment_Data& line1, StreetSegment_Data& line2) ;
std::string getTurnDirection(StreetSegment_Data& prev_seg, StreetSegment_Data& curr_seg);
std::string determineStartingDirection(StreetSegment_Data first_segment, StreetSegment_Data second_segment);
