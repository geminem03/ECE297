#pragma once

const double VERY_ZOOMED_OUT = 25;
const double WHOLE_CITY_ZOOM = 21;
const double PRIMARY_ROADS_VIEW = 22;
const double SECONDARY_ROADS_VIEW = 20;
const double MAIN_ROADS_ONLY = 19;
const double TERTIARY_ROADS_VIEW = 18;
const double CITY_BLOCK = 16;
const double CITY_BLOCK_SUBWAY = 15;
const double RESIDENTIAL_ROADS = 14;
const double LIVING_STREET_VIEW = 12;
const double SINGLE_SUBWAY_VIEW = 13;
const double ALLEY_WAYS = 11;
const double SINGLE_STREET_VIEW = 8;
const double SINGLE_BUILDIG_VIEW = 5;
const double WAY_TOO_ZOOMED = 0;

// Define named constants for line width values
const double MOTORWAY_CITY_BLOCK_WIDTH = 6.5;
const double LINE_WIDTH_CITY_BLOCK = 8;
const double LINE_WIDTH_RESIDENTIAL_ROADS = 7;
const double LINE_WIDTH_ALLEY_WAYS = 14;
const double LINE_WIDTH_SINGLE_STREET_VIEW = 10;
const double LINE_WIDTH_MAIN_ROADS_ONLY = 3.5;
const double LINE_WIDTH_WHOLE_CITY_ZOOM = 2.0;
//
const double LINE_WIDTH_CITY_BLOCK_TO_MAIN_ROADS = 8.0;
const double LINE_WIDTH_RESIDENTIAL_ROADS_TO_CITY_BLOCK = 7.0;
const double LINE_WIDTH_ALLEY_WAYS_TO_RESIDENTIAL_ROADS = 14.0;
const double LINE_WIDTH_SINGLE_STREET_VIEW_TO_ALLEY_WAYS = 10.0;
//
const double LINE_WIDTH_SECONDARY_ROADS_VIEW_TO_PRIMARY_ROADS_VIEW = 1;
const double LINE_WIDTH_TERTIARY_ROADS_VIEW_TO_MAIN_ROADS_ONLY = 2;
const double LINE_WIDTH_CITY_BLOCK_TO_TERTIARY_ROADS_VIEW = 4;
const double LINE_WIDTH_RESIDENTIAL_ROADS_TO_TERTIARY_ROADS_VIEW = 6;
const double LINE_WIDTH_LIVING_STREET_VIEW_TO_RESIDENTIAL_ROADS = 8;
const double LINE_WIDTH_SINGLE_STREET_VIEW_TO_LIVING_STREET_VIEW = 20;
const double LINE_WIDTH_SINGLE_BUILDIG_VIEW_TO_SINGLE_STREET_VIEW = 30;
const double LINE_WIDTH_WAY_TOO_ZOOMED_TO_SINGLE_BUILDIG_VIEW = 40;
//
const double SMALL_ROAD_WIDTH = 1;
const double LIVING_STREET_WIDTH = 3;
const double SINGLE_STREET_WIDTH = 8;
const double WAY_TOO_ZOOMED_WIDTH = 15;
//
const double OTHER_ROAD_WIDTH = 1;
const double SINGLE_OTHER_ROAD_WIDTH = 5;
const double WAY_TOO_ZOOMED_OTHER_ROAD_WIDTH = 10;

//
const double CITY_BLOCK_SUBWAY_WIDTH = 25;
const double CITY_BLOCK_SUBWAY_WIDTH_2 = 20;
const double RESIDENTIAL_SUBWAY_WIDTH = 15;
const double SINGLE_SUBWAY_WIDTH = 10;
const double DEFAULT_SUBWAY_WIDTH = 5;




// sets width and color for motorway and trunks
double motorwayAndTrunk(ezgl::renderer *g, int level);
// sets width and color of highway links
double motorwayAndTrunkLinks(ezgl::renderer *g, int level);
// sets width and color for primary, tertiary, and secondary roads
double primarySecondaryTertiary(ezgl::renderer *g, int level, StreetSegment_Data& segment);
// sets width and color for residential, living street roads
double smallRoads(ezgl::renderer *g, int level);
// sets width and color for all streets that don't have an osm tag value 
double otherRoads(ezgl::renderer *g, int level);
//gets the width of a street based on the zoom level and highway type
double getStreetWidthAndColor(ezgl::renderer *g, int level, StreetSegment_Data& segment);