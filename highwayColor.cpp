#include <string>
#include <iostream>
#include <vector>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include <gtk/gtk.h>
#include <gtk/gtkcomboboxtext.h>
#include "ezgl/camera.hpp"
#include "ezgl/point.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "globals.h"
#include "loadFunctions.h"
#include "drawFunctions.h"
#include "math.h"
#include "highwayColor.h"

double motorwayAndTrunk(ezgl::renderer *g, int level){
   double line_width = 1;
   // range values for the links
   if (level > WHOLE_CITY_ZOOM ) { 
      line_width = LINE_WIDTH_WHOLE_CITY_ZOOM; 
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(MAIN_ROADS_ONLY, WHOLE_CITY_ZOOM, level)) { 
      line_width = LINE_WIDTH_MAIN_ROADS_ONLY;
      g->set_color(PRIMARY_COLOR);
   } 
   else if (inGivenRange(CITY_BLOCK, MAIN_ROADS_ONLY, level)) { 
      line_width = MOTORWAY_CITY_BLOCK_WIDTH;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(RESIDENTIAL_ROADS, CITY_BLOCK, level)) { 
      line_width = LINE_WIDTH_RESIDENTIAL_ROADS;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(ALLEY_WAYS, RESIDENTIAL_ROADS, level)) { 
      line_width = LINE_WIDTH_ALLEY_WAYS;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(SINGLE_STREET_VIEW, ALLEY_WAYS, level))  { 
      line_width = LINE_WIDTH_SINGLE_STREET_VIEW;
      g->set_color(PRIMARY_COLOR);
   }
   return line_width;
}

double motorwayAndTrunkLinks(ezgl::renderer *g, int level){
   double line_width = 1;
   //
   if(inGivenRange(CITY_BLOCK, VERY_ZOOMED_OUT, level)) {g->set_color(BLANK);}
   else if (inGivenRange(CITY_BLOCK, MAIN_ROADS_ONLY, level)) {
      line_width = 0;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(RESIDENTIAL_ROADS, CITY_BLOCK, level)) { 
      line_width = LINE_WIDTH_RESIDENTIAL_ROADS_TO_CITY_BLOCK;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(ALLEY_WAYS, RESIDENTIAL_ROADS, level)) { 
      line_width = LINE_WIDTH_ALLEY_WAYS_TO_RESIDENTIAL_ROADS;
      g->set_color(PRIMARY_COLOR);
   }
   else if (inGivenRange(SINGLE_STREET_VIEW, ALLEY_WAYS, level)) { 
      line_width = LINE_WIDTH_SINGLE_STREET_VIEW_TO_ALLEY_WAYS;
      g->set_color(PRIMARY_COLOR);
   }
   return line_width;
}

double primarySecondaryTertiary(ezgl::renderer *g, int level, StreetSegment_Data& segment){
   double line_width = 1;
   //
   if(inGivenRange(CITY_BLOCK, VERY_ZOOMED_OUT, level) && (segment.highway_tertiary)) {g->set_color(BLANK); }
   else if(inGivenRange(SECONDARY_ROADS_VIEW, PRIMARY_ROADS_VIEW, level))  { 
      line_width = LINE_WIDTH_SECONDARY_ROADS_VIEW_TO_PRIMARY_ROADS_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   else if(inGivenRange(TERTIARY_ROADS_VIEW, MAIN_ROADS_ONLY, level))  { 
      line_width = LINE_WIDTH_TERTIARY_ROADS_VIEW_TO_MAIN_ROADS_ONLY; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(CITY_BLOCK, TERTIARY_ROADS_VIEW, level)) { 
      line_width = LINE_WIDTH_CITY_BLOCK_TO_TERTIARY_ROADS_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(RESIDENTIAL_ROADS, TERTIARY_ROADS_VIEW, level)) { 
      line_width = LINE_WIDTH_RESIDENTIAL_ROADS_TO_TERTIARY_ROADS_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(LIVING_STREET_VIEW, RESIDENTIAL_ROADS, level)) { 
      line_width = LINE_WIDTH_LIVING_STREET_VIEW_TO_RESIDENTIAL_ROADS; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(SINGLE_STREET_VIEW, LIVING_STREET_VIEW, level)) { 
      line_width = LINE_WIDTH_SINGLE_STREET_VIEW_TO_LIVING_STREET_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(SINGLE_BUILDIG_VIEW, SINGLE_STREET_VIEW, level)) { 
      line_width = LINE_WIDTH_SINGLE_BUILDIG_VIEW_TO_SINGLE_STREET_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(WAY_TOO_ZOOMED, SINGLE_BUILDIG_VIEW, level)) { 
      line_width = LINE_WIDTH_WAY_TOO_ZOOMED_TO_SINGLE_BUILDIG_VIEW; 
      g->set_color(ezgl::WHITE);
   }
   return line_width;
}

double smallRoads(ezgl::renderer *g, int level){
   double line_width = SMALL_ROAD_WIDTH;
   //
   if(inGivenRange(RESIDENTIAL_ROADS, WHOLE_CITY_ZOOM, level)) { 
       g->set_color(BLANK);
   }
   else if (inGivenRange(LIVING_STREET_VIEW, RESIDENTIAL_ROADS, level)) { 
       line_width = LIVING_STREET_WIDTH;
       g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(SINGLE_STREET_VIEW, LIVING_STREET_VIEW, level)) { 
       line_width = SINGLE_STREET_WIDTH;
       g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(WAY_TOO_ZOOMED, SINGLE_STREET_VIEW, level)) { 
       line_width = WAY_TOO_ZOOMED_WIDTH;
       g->set_color(ezgl::WHITE);
   }
   return line_width;
}

double otherRoads(ezgl::renderer *g, int level){
   double line_width = OTHER_ROAD_WIDTH;
   //
   if(inGivenRange(LIVING_STREET_VIEW, WHOLE_CITY_ZOOM, level)) { 
       g->set_color(BLANK);
   }
   else if (inGivenRange(SINGLE_STREET_VIEW, LIVING_STREET_VIEW, level) ){ 
       line_width = SINGLE_OTHER_ROAD_WIDTH; 
       g->set_color(ezgl::WHITE);
   }
   else if (inGivenRange(WAY_TOO_ZOOMED, SINGLE_STREET_VIEW, level)) { 
       line_width = WAY_TOO_ZOOMED_OTHER_ROAD_WIDTH; 
       g->set_color(ezgl::WHITE);
   }
   return line_width;
}

double getStreetWidthAndColor(ezgl::renderer *g, int level, StreetSegment_Data& segment){
   double line_width_set = 1;
   // hard coded values for motorway and trunk widths
   if (segment.highway_motorway || segment.highway_trunk){
      line_width_set = motorwayAndTrunk(g, level);
   }
   else if(segment.highway_motorway_link || segment.highway_trunk_link || segment.highway_primary_link){
      line_width_set = motorwayAndTrunkLinks(g, level);
   }
   else if (segment.highway_secondary || segment.highway_tertiary || segment.highway_primary){
      line_width_set = primarySecondaryTertiary(g, level, segment);
   }
   else if(segment.highway_residential || segment.highway_pedestrian || segment.highway_road || segment.highway_livingstreet){
      line_width_set = smallRoads(g, level);
   }
   else{
      line_width_set = otherRoads(g, level);
   }
   return line_width_set;
}