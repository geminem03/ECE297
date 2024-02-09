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
#include "searchFunctions.h"
#include "globals.h"
#include "loadFunctions.h"
#include "drawFunctions.h"
#include "math.h"

const double PADDING = 0.9;

// Function prints the directions for the path found
void printMessage(const std::vector <StreetSegment_Data>& route, ezgl::application* app);

// Call back function for the hide window
void hideWindow(GtkButton* /*self*/, ezgl::application* app);


void openDirections(const char* message, ezgl::application* app);

// Takes in 4 street names, finds its closest intersection and performs a path search on the two intersections
void entered_search(GtkWidget* /*widget*/, ezgl::application* app){
    clearHighlights();
    // create an entry object for the two search bars
    GtkEntry *entry_1 = (GtkEntry*) app->get_object("SearchBar");
    GtkEntry *entry_2 = (GtkEntry*) app->get_object("SearchBar2");
    GtkEntry *entry_3 = (GtkEntry*) app->get_object("SearchBar3");
    GtkEntry *entry_4 = (GtkEntry*) app->get_object("SearchBar4");
    // get the texts in the entry boxes
    const gchar* text_1 = nullptr;
    const gchar* text_2 = nullptr;
    const gchar* text_3 = nullptr;
    const gchar* text_4 = nullptr;
    // if the entries are not null, then get the inputted text
    if (entry_1 != nullptr) {
        text_1 = gtk_entry_get_text(entry_1);
    }
    if (entry_2 != nullptr) {
        text_2 = gtk_entry_get_text(entry_2);
    }
    if (entry_3 != nullptr) {
        text_3 = gtk_entry_get_text(entry_3);
    }
    if (entry_4 != nullptr) {
        text_4 = gtk_entry_get_text(entry_4);
    }
    // find all the streets with the given texts (just in case the user doesn't select from the drop down)
    std::vector<StreetIdx> street_idx_1 = findStreetIdsFromPartialStreetName(noSpacesAndLowercase(text_1));
    std::vector<StreetIdx> street_idx_2 = findStreetIdsFromPartialStreetName(noSpacesAndLowercase(text_2));
    std::vector<StreetIdx> street_idx_3 = findStreetIdsFromPartialStreetName(noSpacesAndLowercase(text_3));
    std::vector<StreetIdx> street_idx_4 = findStreetIdsFromPartialStreetName(noSpacesAndLowercase(text_4));
    // stores the message that gets output to the status bar
    std::string message;
    // store the intersection id's of start and end points 
    int srcID;
    int destID;
    std::vector<StreetSegment_Data> route;
    // stores the street name if vector is not empty
    if (!street_idx_1.empty() && !street_idx_2.empty() && !street_idx_3.empty() && !street_idx_4.empty()){
      // retreives the street id of the first street in the partial names vector
        int id_1 = street_idx_1[0];
        int id_2 = street_idx_2[0];
        int id_3 = street_idx_3[0];
        int id_4 = street_idx_4[0];
        // stores all the intersections between the two entered streets
        std::vector<IntersectionIdx> two_street_intersections_pair1 = findIntersectionsOfTwoStreets(id_1, id_2);
        std::vector<IntersectionIdx> two_street_intersections_pair2 = findIntersectionsOfTwoStreets(id_3, id_4);

        // run find path for streets that have a valid intersection
        if(two_street_intersections_pair1.size() > 0 && two_street_intersections_pair2.size() > 0){
            // take the first intersection regardless of size
            // most will have only one but just for simiplicity take first
            srcID = two_street_intersections_pair1[0];
            destID = two_street_intersections_pair2[0];
            // highlight the start and end intersection 
            intersection_map[srcID].highlight = true;
            intersection_map[destID].highlight = true;
            // get the path of street segments between the two intersections
            std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(std::pair(srcID, destID), 0);
            std::cout<< "Drawing..." << std::endl;
            
            // if path is found, then show path
            if(path.size() > 0){ 
                route = showPath(path, app, srcID, destID);
            }
            // else update message in the status
            else{
                message = "Unable to find a path between the selected intersections.";
            }
        }
        else{
            message = "Invalid intersection input. Check street names.";
        }
    }
    else {
        message = "Streets Not Found";
    }
    // displays message to status bar
    app->update_message(message);
    app->refresh_drawing();
    
}

void searchUponClick(IntersectionIdx from, IntersectionIdx to, ezgl::application* app){  
    std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(std::pair(from, to), 0);
    std::vector<StreetSegment_Data> route = showPath(path, app, from, to);
    // std::cout<< "Starting Point: " << route[0].ss_id << std::endl;
    app->refresh_drawing();
}

std::vector<StreetSegment_Data> showPath(std::vector<IntersectionIdx>& path, ezgl::application* app, IntersectionIdx srcID, IntersectionIdx destID){
    std::vector<StreetSegment_Data> route;
    // iterate through each segment of the path
    for(int seg_num = 0; seg_num < path.size(); seg_num++){
        int ss_id = path[seg_num];
        // highlight segment blue
        street_segments[ss_id].highlight_path = true;
        // get street name for current and previous segment
        std::string street_name = street_segments[ss_id].street_name;
        std::string prev_street_name = "empty";
        
        route.push_back(street_segments[ss_id]);
        
        // print the directions to the status bar 
        // std::cout<< "Segment: " << seg_num << " street name- " << street_name << std::endl;
    }
    //printMessage(route, app);
    // intersection connecting entered streets will now be purple 
    app->flush_drawing();

    // gets the xy values of the src and dest intersections
    ezgl::point2d src_xy = intersection_map[srcID].xy_loc;
    ezgl::point2d dest_xy = intersection_map[destID].xy_loc;

    // get canvas width and height to determine best zoom factor
    // Get the main canvas and its width and height
    // Get the main canvas and its width and height
    // Get the main canvas and its width and height
    auto canvas = app->get_canvas(app->get_main_canvas_id());
    auto& camera = canvas->get_camera();
    double canvas_width = camera.get_initial_world().width();
    double canvas_height = camera.get_initial_world().height();

    // determine the midpoint between the two intersections
    ezgl::point2d midpoint;
    midpoint.x = (src_xy.x + dest_xy.x) / 2.0;
    midpoint.y = (src_xy.y + dest_xy.y) / 2.0;

    // zooms in to the start intersection 
    double zoom_factor = calculateZoomFactor(src_xy, dest_xy, canvas_width, canvas_height);

    zoomInOnPoint(midpoint, app, zoom_factor);
    return route;
}


// void printMessage(const std::vector<StreetSegment_Data>& route, ezgl::application* app) {
//     std::string navigation;
//     if (route.empty()) {
//         //std::cout << "No route found." << std::endl;
//         navigation = "No route found.";
//         const char* message = navigation.c_str();

//         openDirections(message, app);
//     }

//     //Rather than cout, combining all the strings together and feeding it into function 
//     // Print starting direction
//     // Print distance to travel on first street
//     double first_distance = findDistanceBetweenTwoPoints(route[0].from_pos, route[0].to_pos);
//     navigation = determineStartingDirection(route[0], route[1]) + "\n\nContinue straight for " + std::to_string(static_cast<int>(first_distance / 10) * 10) + " m.\n";
//     //std::cout << "\nContinue straight for " << static_cast<int>(first_distance / 10) * 10 << " m. ";
//     // Print directions for each segment of the route
//     int total_distance = 0;
//     StreetSegment_Data prev_seg = route[0];
//     for (size_t i = 1; i < route.size(); ++i) {
//         StreetSegment_Data curr_seg = route[i];
//         std::string turn_direction = getTurnDirection(prev_seg, curr_seg);
//         double distance = findDistanceBetweenTwoPoints(prev_seg.from_pos, prev_seg.to_pos);
//         total_distance += static_cast<int>(distance);
//         if (turn_direction != "") {
//             navigation += "\nTurn " + turn_direction + " onto " + curr_seg.street_name + "\n";
//             //std::cout << "Turn " << turn_direction << " onto " << curr_seg.street_name << ". ";
//             if (total_distance > 0) {
//                 navigation += "\nContinue straight for " + std::to_string(static_cast<int>(total_distance / 10) * 10) + " m.\n";
//                 //std::cout << "Continue straight for " << static_cast<int>(total_distance / 10) * 10 << " m. ";
//             }
//             total_distance = 0;
//         }
//         prev_seg = curr_seg;
//     }
//     // Print the final destination message
//     navigation += "\nYou have arrived at your destination.";
//     const char* message = navigation.c_str();
//     openDirections(message, app);
// }

// // function opens a window for the directions for the users
// void openDirections(const char* message, ezgl::application* app){
//     GtkLabel* label = GTK_LABEL(app->get_object("Path Message"));
//     gtk_label_set_text(label, message);

//     gtk_widget_show_all(GTK_WIDGET(app->get_object("PathWindow")));
    
//     GtkButton* done = (GTK_BUTTON(app->get_object("Done Button")));
//     g_signal_connect(done, "clicked", G_CALLBACK(hideWindow), app);
// }

// void hideWindow(GtkButton* /*self*/, ezgl::application* app){
//     gtk_widget_hide(GTK_WIDGET(app->get_object("PathWindow")));
// }

// creates the drop down for the searched street
void searchDropdownList(ezgl::application* application){
    clearHighlights();
    // Create a GTK list of types string elements only
    GtkListStore *names_list = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter element;

    // create an entry object for the two search bars
    GtkEntry *entry_1 = (GtkEntry*) application->get_object("SearchBar");
    GtkEntry *entry_2 = (GtkEntry*) application->get_object("SearchBar2");
    GtkEntry *entry_3 = (GtkEntry*) application->get_object("SearchBar3");
    GtkEntry *entry_4 = (GtkEntry*) application->get_object("SearchBar4");

    // inputs the street names into the drop box when typing a street name into the search bar
    for (int i = 0; i < streets.size(); i++){
        // Add the completion items from the streets unordered map (contains all street names for the street)
        gtk_list_store_append(names_list, &element);
        const gchar *str_name = (streets[i].street_name_caps).c_str();
        gtk_list_store_set(names_list, &element, 0, str_name, -1);
    }

    // create a entry completion entity and connect it to the 1st search bar
    GtkEntryCompletion *completed_street_1 = gtk_entry_completion_new();
    gtk_entry_completion_set_model(GTK_ENTRY_COMPLETION(completed_street_1), GTK_TREE_MODEL(names_list));
    gtk_entry_completion_set_text_column(GTK_ENTRY_COMPLETION(completed_street_1), 0);
    gtk_entry_set_completion(GTK_ENTRY(entry_1), completed_street_1);

    // create another entry completion entity and connect it to the 2nd search bar
    GtkEntryCompletion *completed_street_2 = gtk_entry_completion_new();
    gtk_entry_completion_set_model(GTK_ENTRY_COMPLETION(completed_street_2), GTK_TREE_MODEL(names_list));
    gtk_entry_completion_set_text_column(GTK_ENTRY_COMPLETION(completed_street_2), 0);
    gtk_entry_set_completion(GTK_ENTRY(entry_2), completed_street_2);


    // create another entry completion entity and connect it to the 3nd search bar
    GtkEntryCompletion *completed_street_3 = gtk_entry_completion_new();
    gtk_entry_completion_set_model(GTK_ENTRY_COMPLETION(completed_street_3), GTK_TREE_MODEL(names_list));
    gtk_entry_completion_set_text_column(GTK_ENTRY_COMPLETION(completed_street_3), 0);
    gtk_entry_set_completion(GTK_ENTRY(entry_3), completed_street_3);

    // create another entry completion entity and connect it to the 4nd search bar
    GtkEntryCompletion *completed_street_4 = gtk_entry_completion_new();
    gtk_entry_completion_set_model(GTK_ENTRY_COMPLETION(completed_street_4), GTK_TREE_MODEL(names_list));
    gtk_entry_completion_set_text_column(GTK_ENTRY_COMPLETION(completed_street_4), 0);
    gtk_entry_set_completion(GTK_ENTRY(entry_4), completed_street_4);
}


// zooms in on highlighted intersections and paths to appropriate level
void zoomInOnPoint(ezgl::point2d point, ezgl::application* app, double zoomFactor) {
   // Get the main canvas and its camera
   auto canvas = app->get_canvas(app->get_main_canvas_id());
   auto& camera = canvas->get_camera();
   // Calculate the new world bounds
   double worldWidth = camera.get_initial_world().width();
   double worldHeight = camera.get_initial_world().height();
   double newWidth = worldWidth / zoomFactor;
   double newHeight = worldHeight / zoomFactor;
   // divide by two to find the midpoint
   double newX = point.x - newWidth / 2;
   double newY = point.y - newHeight / 2;
   ezgl::rectangle newWorld({newX, newY}, {newX + newWidth, newY + newHeight});
   // Set the new world bounds
   camera.set_world(newWorld);
   // Refresh the canvas to display the new view
   app->refresh_drawing();

}

// Function calculates the zoom factor to zoom in on a proper level
double calculateZoomFactor(const ezgl::point2d& src_point, const ezgl::point2d& dest_point, double canvas_width, double canvas_height) {
    // Calculate the distance between the source and destination points
    double distance = std::hypot(src_point.x - dest_point.x, src_point.y - dest_point.y);

    // Calculate the maximum zoom factor that fits both points on the screen
    double max_zoom_factor = std::max(canvas_width / distance, canvas_height / distance);

    // Adjust the zoom factor to provide some padding around the points
    double zoom_factor = max_zoom_factor * PADDING;
    return zoom_factor;
}


// Function for delay
void delay (int milliseconds) { // Pause for milliseconds
    std::chrono::milliseconds duration (milliseconds);
    std::this_thread::sleep_for (duration);
}

// Function clears all highlighted intersections and segments on the map and clears the search bars
void  clearHighlights(){
    int size = intersection_map.size();
    for (int i = 0; i < size; i++){
        intersection_map[i].highlight = false;
    }
    int segment_size = street_segments.size();
    for (int i = 0; i < segment_size; i++){
        street_segments[i].highlight_path = false;
    }
}