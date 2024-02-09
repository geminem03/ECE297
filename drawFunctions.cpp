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

// indicates the current night mode state
bool night_mode = false;
// Indicates the current show poi state
bool show_POI = false;
// indicates the show subway state
bool show_subways = false;

/********************************************************************************/
/********************************Draw Functions**********************************/
/********************************************************************************/

void drawIntersections(ezgl::renderer *g) {
   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // set dimensions for intersections to be 15m on map
   double width = INTERSECTION_WIDTH; 
   double height = width;

   // set color for highlighted intersections
   ezgl::color highlight_color = ezgl::PURPLE;

   // loop through all the intersections in the map
   for(const auto& [inter_id, intersection] : intersection_map) {
      if(!intersection.highlight) {
         // skip unhighlighted intersections
         continue;
      }

      // draw the intersection
      g->set_color(highlight_color);
      g->fill_rectangle(intersection.xy_loc - ezgl::point2d{width/2, height/2}, width, height);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

   std::cout << "drawIntersections took " << elapsedSeconds << " seconds." << std::endl;
}


void drawStreetSegments(ezgl::renderer *g, int level){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // Create a vector to store the street segments in reverse order of precedence
   std::vector<std::vector<StreetSegment_Data>> street_segments_by_priority = streetsByPriority();

   // Loop through the vector in reverse order of precedence and draw the street segments
   for (int priority_index = 10; priority_index >= 0; priority_index--) {
      for (StreetSegment_Data& segment : street_segments_by_priority[priority_index]) {
         // Retrieve the number of curve points that compose the segment
         StreetSegmentInfo seg_info = getStreetSegmentInfo(segment.ss_id);
         int num_curve_points = seg_info.numCurvePoints;

         double line_width = getStreetWidthAndColor(g, level, segment);
         g->set_line_width(line_width);
         // Overwrite the default color to draw path directions
         if(segment.highlight_path){
            g->set_color(ezgl::BLUE);
            g->set_line_width(SINGLE_STREET_WIDTH);
         }
         drawSegmentHelper(g, segment, num_curve_points, segment.curve_points);
      }
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
   std::cout << "drawStreetSegments took " << elapsedSeconds << " seconds." << std::endl;
}



// Draw the street segment using the given renderer, segment data, and number of curve points
void drawSegmentHelper(ezgl::renderer *g, StreetSegment_Data segment, int num_curve_points, std::vector<ezgl::point2d> points){
   // Set the line cap to round
   g->set_line_cap(ezgl::line_cap::round);

   // Get the starting and ending coordinates of the segment
   ezgl::point2d from_loc = segment.from_xy;
   ezgl::point2d to_loc = segment.to_xy;

   // If there are no curve points, draw a straight line and return
   if (num_curve_points == 0){
      g->draw_line(from_loc, to_loc);
      return;
   }

   // Draw a line from the start point to the first curve point
   auto curve = points.begin();
   g->draw_line(from_loc, *curve);

   // Draw lines between all the curve points
   for (auto end = points.end() - 1; curve != end; ++curve) {
      g->draw_line(*curve, *(curve+1));
   }

   // Draw a line from the last curve point to the end point
   g->draw_line(*curve, to_loc);
}

std::vector<std::vector<StreetSegment_Data>> streetsByPriority(){

   // Create a vector to store the street segments in reverse order of precedence
   std::vector<std::vector<StreetSegment_Data>> street_segments_by_priority(11);
   // Loop through all the street segments and add them to the vector according to their priority
   int num_street_segments = getNumStreetSegments();
   for(int ss_id = 0; ss_id < num_street_segments; ss_id++){
      StreetSegment_Data segment = street_segments[ss_id];

      if (segment.highlight_path) {
         // highlight_path=true segments at highest priority
         street_segments_by_priority[0].push_back(segment); 
      } else if (segment.highway_motorway) {
         street_segments_by_priority[1].push_back(segment);
      } else if (segment.highway_trunk) {
         street_segments_by_priority[2].push_back(segment);
      } else if (segment.highway_primary) {
         street_segments_by_priority[3].push_back(segment);
      } else if (segment.highway_secondary) {
         street_segments_by_priority[4].push_back(segment);
      } else if (segment.highway_tertiary) {
         street_segments_by_priority[5].push_back(segment);
      } else if (segment.highway_residential) {
         street_segments_by_priority[6].push_back(segment);
      } else if (segment.highway_road) {
         street_segments_by_priority[7].push_back(segment);
      } else if (segment.highway_pedestrian) {
         street_segments_by_priority[8].push_back(segment);
      } else if (segment.highway_livingstreet) {
         street_segments_by_priority[9].push_back(segment);
      } else {
         // falls into "other" category
         street_segments_by_priority[10].push_back(segment); 
      }
   }
   return street_segments_by_priority;
}


// Draws the names of all the streets
void drawStreetNames(ezgl::renderer *g){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   int level;
   double slope;
   g->format_font("Noto", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
   zoom_levels(g, level);
   // loop through all the street segments to draw their lines
   int num_street_segments = getNumStreetSegments();
   for(int ss_id = 0; ss_id < num_street_segments; ss_id++){
      g->set_font_size(LINE_WIDTH_SINGLE_STREET_VIEW_TO_ALLEY_WAYS);
      // set variables for street name and segment length 
      std::string street_name = street_segments[ss_id].street_name;
      double segment_length = street_segments[ss_id].segment_length;
      // double speed_limit = street_segments[ss_id].speed_limit;
      int num_curve_points = street_segments[ss_id].curve_points.size();
      // set xy variable for intersections that bound street segment
      double x1 = street_segments[ss_id].from_xy.x; 
      double y1 = street_segments[ss_id].from_xy.y; 
      double x2 = street_segments[ss_id].to_xy.x;
      double y2 = street_segments[ss_id].to_xy.y;
      // set variable for the midpoint of the street segment
      ezgl::point2d mid_street_seg = {(x1 + x2)/2, (y1 + y2)/2};
      //draw name of street on segment line for streets > 50 m
      //exclude street segments with unknown street idd
      //if there are no curve points
      if (street_name != "<unknown>" && num_curve_points == 0){
         if (night_mode){//sets street name colour to white on night mode
            g->set_color(ezgl::WHITE);
         }else {
            g->set_color(ezgl::BLACK);
         }
         if (x2 == x1){//takes care of undefined tan values at 90/-90 degrees]
            slope = 90;
            g->set_text_rotation(DEGREES_90);
         } else{
            slope = atan((y2-y1)/(x2-x1)) * DEGREES_180 / PI;
            g->set_text_rotation(slope);
         } 
         //check the ratio of the length of the name and size of the street
         if (street_name.size()/segment_length <0.18){
            g->draw_text(mid_street_seg, street_name, 1*abs(x2-x1), 2*sin(slope)*segment_length);
         } else{ 
            g->draw_text(mid_street_seg, street_name, 1*abs(x2-x1), 2*sin(slope)*segment_length);
         } 
      } else if (street_name != "<unknown>" && num_curve_points > 0) {
         //draws ONLY at the first curve point, could be changed
         for(int i = 0; i < num_curve_points-1; i=i+2){
            // set lead one curve point ahead of tail
            LatLon lead = getStreetSegmentCurvePoint(ss_id, i +1); 
            LatLon trail = getStreetSegmentCurvePoint(ss_id, i);
            // checks size of the curved section (only draws if sufficiently large)
            if (findDistanceBetweenTwoPoints(lead, trail)<10 && num_curve_points > 3){
               //prevents printing on too many curve points
               if(i < num_curve_points-2){
                  lead = getStreetSegmentCurvePoint(ss_id, i +2);
               }
               //get the x and y coords of lead and tail
               x1 = x_from_lon(lead.longitude());
               y1 = y_from_lat(lead.latitude()); 
               x2 = x_from_lon(trail.longitude());
               y2 = y_from_lat(trail.latitude()); 
               mid_street_seg = {(x1 + x2)/2, (y1 + y2)/2};
               if (night_mode){//white street names for night mode
                  g->set_color(ezgl::WHITE);
               }else {
                  g->set_color(ezgl::BLACK);
               }
               slope = atan((y2-y1)/(x2-x1)) * 180 / 3.1415;//defines the slope to print at
               if (x2 == x1){
                  slope = 90;
                  g->set_text_rotation(90);//same as above
               } else{
                  g->set_text_rotation(slope);
               } 
               //writes the street name on the street segment
               if (street_name.size()/segment_length <0.1){
                  g->draw_text(mid_street_seg, street_name, 3*abs(x2-x1), 3*sin(slope)*segment_length);
               } else{ 
                  g->draw_text(mid_street_seg, street_name, 2*abs(x2-x1), 2*sin(slope)*segment_length);
               }
            }
         }
      }
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "drawStreetNames took " << elapasedTime.count() << "seconds." <<std::endl;

}

// function compares the two passed areas
bool compareFeatureArea(const FeatureArea& f1, const FeatureArea& f2) {
   return f1.area > f2.area;
}

// Function draws the features on the canvas
void drawFeatures(ezgl::renderer *g) {

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   int num_features = getNumFeatures();
   int level;
   zoom_levels(g, level);

   std::vector<FeatureArea> feature_areas;
   feature_areas.reserve(num_features);

   // loop through all the closed polygon features on the map
   for (int feat_id = 0; feat_id < num_features; feat_id++) {
      if (features[feat_id].is_closed_polygon) {
         int num_feat_points = getNumFeaturePoints(feat_id);
         if (num_feat_points > 1) {
            double area = findFeatureArea(feat_id);
            feature_areas.push_back({feat_id, area});
         }
      }
   }

   std::sort(feature_areas.begin(), feature_areas.end(), compareFeatureArea);

   // depending on the features, specific colors are set, while taking into account if night mode is enabled
   for (const auto& feature : feature_areas) {
      int feat_id = feature.id;
      std::vector<ezgl::point2d> feature_point_xy = features[feat_id].feature_point_xy;
      FeatureType feature_type = features[feat_id].feature_type;

      switch (feature_type) {
         case PARK:
         case GREENSPACE:
         case GOLFCOURSE:
            g->set_color(night_mode ? dark_green : light_green);
            break;
         case LAKE:
         case RIVER:
            g->set_color(night_mode ? night_blue : light_blue);
            break;
         case BEACH:
            g->set_color(night_mode ? beach_grey : light_green);
            break;
         case ISLAND:
            g->set_color(night_mode ? beach_grey : background_color);
            break;
         case BUILDING:
            if (level < TERTIARY_ROADS_VIEW) {
               g->set_color(night_mode ? ezgl::PURPLE : light_grey);
            } 
            else {
               continue;
            }
            break;
         case GLACIER:
            g->set_color(night_mode ? night_glacier : glacier);
            break;
         default:
            g->set_color(ezgl::BLACK);
            break;
      }
      g->fill_poly(feature_point_xy);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "drawFeatures took " << elapasedTime.count() << "seconds." <<std::endl;

}


// Draws the name of a POI at the centroid of its shape
void drawPOINames(ezgl::renderer *g){
   int num_features = getNumFeatures();
   double font_size = POI_FONT_SIZE;
   // loops through all the features on the map 
   for(int feat_id = 0; feat_id < num_features; feat_id++){
      // retrieves the vector with the feature vertices
      std::vector<ezgl::point2d> feature_point_xy = features[feat_id].feature_point_xy;
      int num_feat_points = getNumFeaturePoints(feat_id);
      // calculates the centroid of the feature polygon
      ezgl::point2d centroid = compute2DPolygonCentroid(feature_point_xy, num_feat_points);
      // sets variables for feature name and type
      std::string name = features[feat_id].feature_name;
      // FeatureType feature_type = features[feat_id].feature_type;
      // draws the name for POIs
      if(name != "<noname>"){
         g->set_color(ezgl::BLACK);
         g->format_font("Noto", ezgl::font_slant::normal, ezgl::font_weight::normal, font_size);
         g->draw_text(centroid, name, font_size,   font_size);
      }
   }
}

// Function gets the bounds of the features in rectangle coordinates
ezgl::rectangle getFeatureBounds(int feature_id) {
    int num_points = getNumFeaturePoints(feature_id);

    for (int i = 0; i < num_points; i++) {
        LatLon point = getFeaturePoint(feature_id, i);
        double lon = x_from_lon(point.longitude());
        double lat = y_from_lat(point.latitude());
        min_lon = std::min(min_lon, lon);
        max_lon = std::max(max_lon, lon);
        min_lat = std::min(min_lat, lat);
        max_lat = std::max(max_lat, lat);
    }

    return {ezgl::point2d{min_lon, min_lat}, ezgl::point2d{max_lon, max_lat}};
}

// check if two rectangles intersect with one another for drawing purposes
bool intersects(const ezgl::rectangle& rect1, const ezgl::rectangle& rect2) {
  return !(rect1.right() < rect2.left() || rect2.right() < rect1.left() || rect1.top() < rect2.bottom() || rect2.top() < rect1.bottom());
}

// Focus on pedestrian safety so icons are for city resources, emergency services 
// Not sure if should do it through POI or osm like done with highway
void drawPOIIcons(ezgl::renderer *g){
   // vector is POIs
   double scale = 1;
   for (int i = 0; i < POIs.size(); i++){
      // if poi is hospital, show hospital pin point
      if(POIs[i].POI_type == "hospital" ){
         ezgl::surface* icon = ezgl::renderer::load_png("libstreetmap/resources/hospital_1.png");
         g->draw_surface(icon, POIs[i].POI_xy, scale);
         ezgl::renderer::free_surface(icon);
         // if poi is pharmacy, show pharmacy pin point icon
      } else if(POIs[i].POI_type == "pharmacy"){
         ezgl::surface* icon = ezgl::renderer::load_png("libstreetmap/resources/pharmacy_1.png");
         g->draw_surface(icon, POIs[i].POI_xy, scale);
         ezgl::renderer::free_surface(icon);
         // if poi is bus-station, show bus_station icon
      } else if(POIs[i].POI_type == "bus_station"){
         ezgl::surface* icon = ezgl::renderer::load_png("libstreetmap/resources/bus_stop_1.png");
         g->draw_surface(icon, POIs[i].POI_xy, scale);
         ezgl::renderer::free_surface(icon);
      }
   }
}

// Finds the segment in the vector of structs and returns the first occurance
StreetSegment_Data findSegmentData(StreetSegmentIdx seg){
   StreetSegment_Data segment;
   for (int i = 0; i < street_segments.size(); i++){
      StreetSegment_Data curr_seg = street_segments[i];
      if(curr_seg.ss_id == seg){
         segment = curr_seg;
         break;
      }
   }
   return segment;
}