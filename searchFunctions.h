#pragma once

#include "globals.h"


// A callback function that acts on the search bar and searches for streets
void entered_search(GtkWidget* /*widget*/, ezgl::application* app);
// provides a list of streets for the search bar when user is typing the street name
void searchDropdownList(ezgl::application* application);
// implemented search bar for intersection for m3 debugging 
void intersection_search(GtkWidget* /*widget*/, ezgl::application* app);
// helper function that prints out the directions message depending on the segment
void printMessage(const std::vector<StreetSegment_Data>& route);
// zooms in on highlighted intersections and paths to appropriate level
void zoomInOnPoint(ezgl::point2d point, ezgl::application* app, double zoomFactor);
// Function calculates the zoom factor
double calculateZoomFactor(const ezgl::point2d& src_point, const ezgl::point2d& dest_point, double canvas_width, double canvas_height);
// Function clears all the highlights on the map
void clearHighlights();
// Function creates the delay for the given time
void delay (int milliseconds);
// Call back function for the searching a path on a rightclick
void searchUponClick(IntersectionIdx from, IntersectionIdx to, ezgl::application* app);
// Function draws the path on the canvas
std::vector<StreetSegment_Data> showPath(std::vector<IntersectionIdx>& path, ezgl::application* app, IntersectionIdx srcID, IntersectionIdx destID);