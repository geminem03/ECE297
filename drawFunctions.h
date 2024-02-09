#pragma once

const double POI_FONT_SIZE = 50;
const double DEGREES_90 = 90;
const double DEGREES_180 = 180;
const double INTERSECTION_WIDTH = 10;

// This function draws all the intersections
void drawIntersections(ezgl::renderer *g);
// This function draws the curve points that compose a street segment
void drawStreetSegments(ezgl::renderer *g, int level);
// This function is a helper function to draw the curve points for a given street segment
void drawSegmentHelper(ezgl::renderer *g, StreetSegment_Data segment, int num_curve_points, std::vector<ezgl::point2d> points);
// This function draws the names of all the streets
void drawStreetNames(ezgl::renderer *g);
// This function is a helper function to draw and fill map features
void drawFeatures(ezgl::renderer *g);
// Returns the bounds of the given feature
ezgl::rectangle getFeatureBounds(int feature_id);
//Checks if the two given rectangle areas intersect
bool intersects(const ezgl::rectangle& rect1, const ezgl::rectangle& rect2);
// Allows comparison of two different areas
bool compareFeatureArea(const FeatureArea& f1, const FeatureArea& f2);
// This function draws the name of a POI at the centroid of its shape
void drawPOINames(ezgl::renderer *g);
// This function draws the surrounding polygons of a street segment
void drawSurroundingPolygons(ezgl::renderer* g, StreetSegmentIdx ss_id);
// Draws the point of interest ont map
void drawPOIIcons(ezgl::renderer *g);
// Finds the segment data from the data structure
StreetSegment_Data findSegmentData(StreetSegmentIdx seg);
// organize streets with OSM order of precidance 
std::vector<std::vector<StreetSegment_Data>> streetsByPriority();