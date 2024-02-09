
#include <chrono>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>


#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/color.hpp"
#include <gtk/gtk.h>
#include <gtk/gtkcomboboxtext.h>
#include "color.hpp"



#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "globals.h"
#include "math.h"

double x_from_lon(double  lon){
   double x = lon * kDegreeToRadian * kEarthRadiusInMeters * cos(kDegreeToRadian * avg_lat);
   return x;
}
double y_from_lat(double lat){
   double  y = kEarthRadiusInMeters * lat * kDegreeToRadian;
   return y;
}
double lon_from_x(double x){
   double lon = x/(kDegreeToRadian * kEarthRadiusInMeters * cos(kDegreeToRadian * avg_lat));
   return lon;
}
double lat_from_y(double y){
   double lat = y/(kEarthRadiusInMeters * kDegreeToRadian);
   return lat;
}


// computes the centroid of a feature polygon
// copied from stackoverflow example with a few adjustments to the input parameters
ezgl::point2d compute2DPolygonCentroid(std::vector<ezgl::point2d> &vertices, int vertexCount){
   ezgl::point2d centroid = {0, 0};
   double signedArea = 0.0;
   // current xy verticies
   double x0 = 0.0; 
   double y0 = 0.0; 
   // next xy verticies
   double x1 = 0.0;
   double y1 = 0.0; 
   // Partial signed area
   double a = 0.0; 
   // For all vertices
   int i = 0;
   for (i = 0; i < vertexCount; ++i){
      x0 = vertices[i].x;
      y0 = vertices[i].y;
      x1 = vertices[(i+1) % vertexCount].x;
      y1 = vertices[(i+1) % vertexCount].y;
      a = (x0 * y1) - (x1 * y0);
      signedArea += a;
      centroid.x += (x0 + x1) * a;
      centroid.y += (y0 + y1) * a;
   }
   signedArea *= 0.5;
   centroid.x /= (6.0 * signedArea);
   centroid.y /= (6.0 * signedArea);
   return centroid;
}

void zoom_levels(ezgl::renderer *g, int& level) {
    
   ezgl::rectangle rec = g->get_visible_world();
    
   double x1 = rec.m_first.x;
   double y1 = rec.m_first.y;
   double x2 = rec.m_second.x;
   double y2 = rec.m_second.y;
    
   double x = x2 - x1;
   double y = y2 - y1;
   double area = x * y;
    
   //use natural log of area to define zoom level
   level = log(area);
}

bool inGivenRange(int low, int high, int num){
   return ((num-high)*(num-low) <= 0);
}

// Calculates the angle between two StreetSegment_Data objects.
double angleBetween2Lines(StreetSegment_Data& line1, StreetSegment_Data& line2) {
   // Calculate the vectors between the 'from' and 'to' coordinates for each StreetSegment_Data object.
   double x1 = line1.to_pos.longitude() - line1.from_pos.longitude();
   double y1 = line1.to_pos.latitude() - line1.from_pos.latitude();
   double x2 = line2.to_pos.longitude() - line2.from_pos.longitude();
   double y2 = line2.to_pos.latitude() - line2.from_pos.latitude();
   // Calculate the dot product and magnitudes of the vectors.
   double dotProduct = x1 * x2 + y1 * y2;
   double mag1 = sqrt(x1 * x1 + y1 * y1);
   double mag2 = sqrt(x2 * x2 + y2 * y2);
   // Calculate the angle between the two vectors using the dot product.
   double angle = acos(dotProduct / (mag1 * mag2));
   // Convert the angle from radians to degrees.
   angle *= 180 / PI;
   // Check the sign of the angle using the cross product of the vectors.
   double crossProduct = x1 * y2 - x2 * y1;
   if (crossProduct < 0) {
      angle = -angle;
   }
   // Return the angle between the two StreetSegment_Data objects.
   return angle;
}

std::string getTurnDirection(StreetSegment_Data& prev_seg, StreetSegment_Data& curr_seg){
   // calculate the angle between the previous and current street segments
   double angle = angleBetween2Lines(prev_seg, curr_seg);
   // determine whether to turn left or right based on the sign of the angle
   std::string turn_direction;
   if (angle > 0) {
      turn_direction = "left";
   } else if (angle < 0) {
      turn_direction = "right";
   }
   // only return turn direction if the street name changes
   if(prev_seg.street_name != curr_seg.street_name) {
        return turn_direction;
   } else {
        return "";
   }
}

std::string determineStartingDirection(StreetSegment_Data first_segment, StreetSegment_Data second_segment) {
   // Calculate the angle between the two segments
   double angle = angleBetween2Lines(first_segment, second_segment);
    
   // Determine the starting direction based on the angle
   if (angle >= -45 && angle < 45) {
      return "Start driving on " + first_segment.street_name + " heading east";
   } else if (angle >= 45 && angle < 135) {
      return "Start driving on " + first_segment.street_name + " heading north";
   } else if (angle >= 135 || angle < -135) {
      return "Start driving on " + first_segment.street_name + " heading west";
   } else {
      return "Start driving on " + first_segment.street_name + " heading south";
   }
}