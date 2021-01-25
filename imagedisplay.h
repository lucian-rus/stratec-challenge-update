#pragma once
#ifndef IMAGEDISPLAY_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

/******************* macros *********************/

// general use macros
#define OK     1
#define NOT_OK 0

#define SIZE_MULTIPLIER     10
#define BOUNDARY_CORRECTION  2

// macro representing the desired level
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4

// macros used for the basic colors
#define WHITE  cv::Scalar(255, 255, 255)
#define BLACK  cv::Scalar(  0,   0,   0)
#define BLUE   cv::Scalar(255,   0,   0)
#define GREEN  cv::Scalar(  0, 255,   0)
#define RED    cv::Scalar(  0,   0, 255)

// macros used for the field representation 
#define ENTITY_C cv::Scalar(152, 255, 152)
#define AXIS_C   cv::Scalar(220, 220, 220)
#define FIELD_C  cv::Scalar(105, 105, 105)
#define WINDOW   "stratec-update-challenge"

// macros used for the color detection range
#define LOWER_LIMIT cv::Scalar(60, 100, 100)
#define UPPER_LIMIT cv::Scalar(60, 255, 255)

// macros used for preprocessing the image
#define BLUR    1
#define TO_HSV  2
#define ISOLATE 3

// macros used to enable or disable visual debugging
#define DEBUG_ON  1
#define DEBUG_OFF 0

// macros used as formatters
#define FORMAT_BASICS "(%d, %d) W: %d, H: %d\n"

// error macros
#define PREPROCESS_ERROR "error while preprocessing image"

/***************** structures *******************/

struct ENTITY
{
	// stores the id of the entity's countour
	int cont_id;
	// stores the properties of the entity
	std::tuple<int, int, int, int> body;
	// stores a cropped version of the entity
	cv::Mat shape;
};

/***************** functions *******************/

/**
 * This function is used to output the global OUTPUT cv::Mat object
 * 
 */
void display_image();

/**
* This function is used to display the field data in the debug console
*
*/
void debug_field_data();

/**
 * This function is used to retrieve the field data from the input manager
 *
 * @param
 * 
 * @return
 */
int init_field_data(int level);

/**
 * This function is used to create the field based on the collected data
 *
 * @return
 */
cv::Mat update_ui_field();

/**
 * This function is used to preprocess the cv::Mat object and allows overwriting the global 
 * OUTPUT object to display the steps that are taking place inside it (visual debugging)
 * 
 * @param
 * @param
 * @param
 * 
 * @return
 */
int preprocess_mat(cv::Mat target, int flag, int debug);

/**
 * This function is used to get the contours based on the preprocessed cv::Mat object as 
 * well as solving the given challenges by getting the number of entities, their properties
 * and other useful information
 * 
 * @param
 * @param
 * 
 * @return 
 */
int process_mat(cv::Mat target, cv::Mat output);

/**
 * This function is used as the entry point to the imagedisplay.cpp file
 * 
 * @param
 * @param
 */
void id_entry_point(int selected_level, cv::Mat target);

/**
 * This function handles the console input, for the moment. It's just a placeholder for the
 * future implementation
 * 
 */
void console_command();

#endif