
/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
#include "m3.h"
#include "globals.h"
#include "loadFunctions.h"
#include "drawFunctions.h"
#include "math.h"
#include "searchFunctions.h"


/********************************************************************************/
/******************************Function Declaration******************************/
/********************************************************************************/

// draws the ezgl mapper canvas and calls the draw functions
void drawMainCanvas(ezgl::renderer *g);
// holds the callback functions for all the gtk buttons
void initial_setup(ezgl::application *app, bool /*new_window*/);
// highlights intersections when clicked
void act_on_mouse_click(ezgl::application *application, GdkEventButton *event, double x, double y);
// function checks if a key is press and performs the specific code accordingly
void act_on_key_press(ezgl::application* application, GdkEventKey*event, char *key_name);
// calls all the draw functions and is called by drawMap
void loadHelperFunctions();
// Sets the map to a dark color scheme suitable for night viewing
void nightMode(ezgl::renderer *g); 
// A callback function that toggles the night mode state when called
void toggle_night_mode(GtkSwitch* , gboolean night_mode_on, ezgl::application* app);
// A callback function that toggles the subway stations and routes on the map
void drawSubwayLinesCallback(GtkSwitch* /*self*/, gboolean showSubwayLines, ezgl::application* app);

// measures the frames per seconds (FPS) of the gis
void FPS();
//changes the map when a new map is selected on the drop down menu
void changeMap(GtkWidget* widget, gpointer data);
//turns POIs on or off when the switch is flipped
void toggle_POIs(GtkSwitch* /*self*/, gboolean show_poi, ezgl::application* app);
//creates a popup window with instructions for our map.
void help(GtkButton* /*self*/, ezgl::application* app);
// Clears all the highlighted intersections and segments and clears the entru bars
void clear(GtkButton* /*self*/, ezgl::application* app);
// Clears all the search entries
void clearSearchEntry(ezgl::application* app);



/********************************************************************************/
/******************************Global Variables**********************************/
/********************************************************************************/

// A vector that stores the spatial data of intersections 
std::vector<Intersection_data> intersections;
// A vector that stores the data pertinent to street segment drawings
std::vector<StreetSegment_Data> street_segments;
// A vector that stores the data for all the features on the map
std::vector<Feature_Data> features;
// A vector that stores the data for all POIs on the map 
std::vector<POIData> POIs;
// A vector that stores the name and colour of all the subway stations 
std::vector<const OSMNode*> osmSubwayStations;
// Stores all the subway lines and its associated information
std::vector<SubwayLine> subway_lines_info; 
// stores all the points of a street as point2d
std::unordered_map <StreetIdx, std::vector<ezgl::point2d>> street_points;
// Global variables to define the canvas size to fit the city 
double max_lat;
double min_lat;
double max_lon;
double min_lon;
int numOfRightClicks = 0;
IntersectionIdx from_intersection, to_intersection;

// Holds all the intersection and its data
std::unordered_map<IntersectionIdx, Intersection_data> intersection_map;
ezgl::rectangle initial_world;

std:: unordered_map<std::string, std::string> maps = {
   {"Beijing", "beijing_china"},
   {"Cairo", "cairo_egypt"},
   {"Golden Horseshoe", "golden-horseshoe_canada"},
   {"Iceland", "iceland"},
   {"Interlaken", "interlaken_switzerland"},
   {"London", "london_england"},
   {"New Delhi", "new-delhi_india"},
   {"Rio de Janerio", "rio-de-janeiro_brazil"},
   {"Saint Helena", "saint-helena"},
   {"Tehran", "tehran_iran"},
   {"Tokyo", "tokyo_japan"},
   {"Toronto", "toronto_canada"},
};


/********************************************************************************/
/***********************************Draw Map************************************/
/********************************************************************************/

void drawMap() {

   loadHelperFunctions();
   ezgl::application::settings settings;
   // main.ui defines window layout 
   settings.main_ui_resource = "libstreetmap/resources/main.ui";
   settings.window_identifier = "MainWindow";
   settings.canvas_identifier = "MainCanvas";

   // creates the EZGL application
   ezgl::application application(settings);
   // defines a canvas area to draw upon in xy coordinates
   initial_world = {
      {x_from_lon(min_lon), y_from_lat(min_lat)}, 
      {x_from_lon(max_lon), y_from_lat(max_lat)}};
   // parameters are: location, co-ordinate system, and callback function
   application.add_canvas("MainCanvas", drawMainCanvas, initial_world);
   // passes control to EZGL and opens graphics window 
   application.run(initial_setup, act_on_mouse_click, nullptr, act_on_key_press);

}

void loadHelperFunctions(){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   loadIntersectionData();
   std::cout << "--intersection data loaded---" << std::endl;
   loadStreetSegmentData();
   std::cout << "--street segment data loaded---" << std::endl;
   loadFeatureData();
   std::cout << "--feature data loaded---" << std::endl;
   loadPOIData();
   std::cout << "--POI data loaded---" << std::endl;
   loadSubwayOSMValues();
   loadStreetPoints();

   std::cout << "--Subway data loaded---" << std::endl;

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "loadHelperFunctions took " << elapasedTime.count() << "seconds." <<std::endl;
}

// Finds the fps of the rendering
void FPS() {
   // Define a static variable to keep track of the last time the function was called
   static std::chrono::time_point<std::chrono::steady_clock> last_time;
   // Get the current time
   auto current_time = std::chrono::steady_clock::now();
   // Calculate the time difference between the current time and the last time the function was called
   auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
   // Calculate the FPS based on the time difference
   double fps = MILLISECONDS_PER_SECOND / time_diff;
   // Update the last time to the current time
   last_time = current_time;

   // Print the FPS to the console
   std::cout << "FPS: " << fps << std::endl;
}

// called by application.run() to draw the canvas
// calls all the helper functions to draw map features
void drawMainCanvas(ezgl::renderer *g){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   //Define zoom levels 
   int level;
   zoom_levels(g, level);

   // set background color to grey
   g->set_color(background_color);
   ezgl::point2d bottom_left{g->get_visible_world().left(), g->get_visible_world().bottom()};
   ezgl::point2d top_right{g->get_visible_world().right(), g->get_visible_world().top()};
   g->fill_rectangle(bottom_left, top_right);

   // If night mode is on, fill the visible world with dark gray color
   if(night_mode){
      g->set_color(night_grey);
      g->fill_rectangle(g->get_visible_world());
   }
   // Draw features
   drawFeatures(g);
   // Draw street segments
   drawStreetSegments(g, level);
   // Draw intersections when searched for
   drawIntersections(g);
   // Draw street names
   drawStreetNames(g);

   if(show_POI){
      drawPOIIcons(g);
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "drawMainCanvas took " << elapasedTime.count() << "seconds." <<std::endl;

}



/********************************************************************************/
/********************************Helper Functions********************************/
/********************************************************************************/

void initial_setup(ezgl::application *app, bool /*new_window*/){ 

   // create an entry object for the two search bars
   // Create the entry objects and connect the signal of the entry to the  function
   GtkEntry *entry_1 = (GtkEntry*) app->get_object("SearchBar");
   GtkEntry *entry_2 = (GtkEntry*) app->get_object("SearchBar2");
   GtkEntry *entry_3 = (GtkEntry*) app->get_object("SearchBar3");
   GtkEntry *entry_4 = (GtkEntry*) app->get_object("SearchBar4");
   
   g_signal_connect(entry_1, "activate", G_CALLBACK(entered_search), app);
   g_signal_connect(entry_2, "activate", G_CALLBACK(entered_search), app);
   g_signal_connect(entry_3, "activate", G_CALLBACK(entered_search), app);
   g_signal_connect(entry_4, "activate", G_CALLBACK(entered_search), app);


   // callback sequence for night mode switch 
   GObject *nightMode = app->get_object("Night Mode Switch");
   g_signal_connect(nightMode, "state-set", G_CALLBACK(toggle_night_mode), app);

   // callback sequence for find directions feature 
   GObject *directions = app->get_object("Directions Button");
   g_signal_connect(directions, "clicked", G_CALLBACK(entered_search), app);

   // callback sequence for POIs
   GObject *displayPOI = app->get_object("POI Switch");
   g_signal_connect(displayPOI, "state-set", G_CALLBACK(toggle_POIs), app);

   GObject *clear_btn = app->get_object("clear");
   g_signal_connect(clear_btn, "clicked", G_CALLBACK(clear), app);
   
   //callback for the map changing combo box (Drop down menu)
   GObject *mapSelect = app->get_object("comboBxMp");
   g_signal_connect(mapSelect, "changed", G_CALLBACK(changeMap), app);

   //call back for the help window button
   GObject *Help = app->get_object("Help Button");
   g_signal_connect(Help, "clicked", G_CALLBACK(help), app);

}

// Highlights the intersection on mouse click and displays information in the terminal
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // set variable for LatLon of where mouse is clicked 
   LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
   if (event->button == 1){ // left click check
      // determine closest intersection from the clicked LatLon value 
      IntersectionIdx selected_intersection = findClosestIntersection(pos);
      // set the highlight state to retain filled in intersection with map refreshes
      if(intersection_map[selected_intersection].highlight != true){
         intersection_map[selected_intersection].highlight = true;
         std::string message = "Intersection Selected: " + getIntersectionName(selected_intersection) + "  " + std::to_string(selected_intersection);
         std::cout << event << std::endl;
         // output message to status bar at bottom of graphics window 
         app->update_message(message);
      } else{
         intersection_map[selected_intersection].highlight = false;
         std::string message = "Unselected intersection: " + getIntersectionName(selected_intersection) + "  " + std::to_string(selected_intersection);
         app->update_message(message);
      }

      // refreshed the map image with changed state for highlight 
      app->refresh_drawing();
   }
   else if (event->button == 3){ // right click check
      clearSearchEntry(app);
      numOfRightClicks += 1;
      std::string message;
      
      // check if the number of right click is 1
      if (numOfRightClicks == 1){
         // set the selected intersection as from intersection
         from_intersection = findClosestIntersection(pos);
         intersection_map[from_intersection].highlight = true;
         message = "Selected 'from' Intersection: " + getIntersectionName(from_intersection) + "  " + std::to_string(from_intersection);
         app->update_message(message);
         // check if the user right clicked another location, if true, then set that selected intersection
         // as the to intersection
      } else if (numOfRightClicks == 2){
         to_intersection = findClosestIntersection(pos);
         intersection_map[to_intersection].highlight = true;
         message = "Selected 'to' Intersection: " + getIntersectionName(to_intersection) + "  " + std::to_string(to_intersection);
         app->update_message(message);
      } 
      app->refresh_drawing();
      // if number of clicks are two
      if(numOfRightClicks == 2){
         searchUponClick(from_intersection, to_intersection, app);
      } else if (numOfRightClicks > 2){
         message = "Cannot select more than two intersections.";
         app->update_message(message);
         numOfRightClicks =  0;
         clearHighlights();
      }
   }

   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "act_on_mouse_click took " << elapasedTime.count() << "seconds." <<std::endl;
}

// Checks if the user presses any keys 
void act_on_key_press(ezgl::application* application, GdkEventKey* /*event*/, char *key_name){
   searchDropdownList(application);
   std::cout<< "Key Pressed: " << key_name << std::endl;
}

void toggle_night_mode(GtkSwitch* /*self*/, gboolean night_mode_on, ezgl::application* app){
   if(night_mode_on)
      night_mode = true;
   else  
      night_mode = false;
   app->refresh_drawing();
}

void clearDatabases(){

   intersections.clear();
   street_segments.clear();
   features.clear();
   POIs.clear();
   subway_lines_info.clear();
   osmSubwayStations.clear();
   street_points.clear();
   segment_speedLimits.clear();
   OSMid_Nodes.clear();
   OSMid_Ways.clear();
   maps.clear();
   intersection_map.clear();

   intersections.shrink_to_fit();
   street_segments.shrink_to_fit();
   features.shrink_to_fit();
   POIs.shrink_to_fit();
   subway_lines_info.shrink_to_fit();
   osmSubwayStations.shrink_to_fit();
   segment_speedLimits.shrink_to_fit();

}

//changes the map when the user selects a new map from drop down menu 
void changeMap(GtkWidget* widget, gpointer data){

   // stores the time when the function began
   auto startTime = std::chrono::high_resolution_clock::now();

   // Cast the data pointer to a renderer object
   ezgl::application* app = static_cast<ezgl::application*>(data);
   const gchar *selected_item = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
   std::string map_name = selected_item;
   //making sure the user selects a new map (could create a pop up box here instead)
   if (selected_item == nullptr ||  map_name ==  "Select Map"){
      std::cout << "Map is not selected" << std::endl;
   } else {
      //for example "/cad2/ece297s/public/maps/Toronto_Canada.streets.bin"
      std::string map_path = "/cad2/ece297s/public/maps/"+ maps[map_name] +".streets.bin";
      std::cout << map_path << std::endl;
      closeMap();
      
      std::cout << map_path << std::endl;
      bool load = loadMap(map_path);
      loadHelperFunctions();
      
      if (load == true){
         initial_world = {{x_from_lon(min_lon), y_from_lat(min_lat)}, {x_from_lon(max_lon), y_from_lat(max_lat)}};
         app->change_canvas_world_coordinates("MainCanvas", initial_world);
         app->refresh_drawing();

      }
   }
   //reinitialize maps after clearing all datastructures in closeMap
   maps = {
      {"Beijing", "beijing_china"},
      {"Cairo", "cairo_egypt"},
      {"Golden Horseshoe", "golden-horseshoe_canada"},
      {"Iceland", "iceland"},
      {"Interlaken", "interlaken_switzerland"},
      {"London", "london_england"},
      {"New Delhi", "new-delhi_india"},
      {"Rio de Janerio", "rio-de-janeiro_brazil"},
      {"Saint Helena", "saint-helena"},
      {"Tehran", "tehran_iran"},
      {"Tokyo", "tokyo_japan"},
      {"Toronto", "toronto_canada"},
   };


   // calculates the elapsed time for the function to run
   auto endTime = std::chrono::high_resolution_clock::now();
   auto elapasedTime = 
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
   std::cout << "changeMap took " << elapasedTime.count() << "seconds." <<std::endl;

}

//The callback function when the POI switch is toggled
void toggle_POIs(GtkSwitch* /*self*/, gboolean show_poi, ezgl::application* app){
   if(show_poi){
      show_POI = true;
   } else {
      show_POI = false;
   }
   app->refresh_drawing();
}

//Creates a new window with instructions when the help button is called
void help(GtkButton* /*self*/, ezgl::application* app){
   GObject *helpWindow = app->get_object("HelpWindow");
   GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
   GtkWidget* help_message = gtk_message_dialog_new (GTK_WINDOW(helpWindow), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "This map is designed with your safety in mind. \n\nFor directions from intersection A to intersection B, please enter one street name of intersection A in the top left text box and the second in the top right box. \nFor example, if intersection A is Bloor Street and Yonge Street, type Bloor Street into the top left and Yonge Street into the text box next to it. Do the same for intersection B using the text boxes below and press 'Find Directions'. \nAlternatively, you can press on the desired intersections (marked with purple boxes) to find the quickest path between them.\n\nIf you prefer a darker colour scheme, toggle our 'Night Mode' Switch to change the colours. \nIf you prefer a cleaner map without the various location names and symbols, toggle the 'POI's' switch. \nIn order to change to a map of a different city, open the dropdown box that is initialized as 'Select Map' and change the map to the city of your choice.");
   gtk_dialog_run (GTK_DIALOG (help_message));
   gtk_widget_destroy (help_message);
}

// Clears all highlighted areas on the map when the button is clicked
void clear(GtkButton* /*self*/, ezgl::application* app){
   numOfRightClicks = 0;
   clearSearchEntry(app);
   clearHighlights();
   app->refresh_drawing();
}

// Function clears the search entries
void clearSearchEntry(ezgl::application* app){
   GtkEntry *entry_1 = (GtkEntry*) app->get_object("SearchBar");
   GtkEntry *entry_2 = (GtkEntry*) app->get_object("SearchBar2");
   GtkEntry *entry_3 = (GtkEntry*) app->get_object("SearchBar3");
   GtkEntry *entry_4 = (GtkEntry*) app->get_object("SearchBar4");
   gtk_entry_set_text(GTK_ENTRY(entry_1), "");
   gtk_entry_set_text(GTK_ENTRY(entry_2), "");
   gtk_entry_set_text(GTK_ENTRY(entry_3), "");
   gtk_entry_set_text(GTK_ENTRY(entry_4), "");
}