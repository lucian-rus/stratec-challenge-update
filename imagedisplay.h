#pragma once
#ifndef IMAGEDISPLAY_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

/******************* macros *********************/

#define SIZE_MULTIPLIER     10
#define BOUNDARY_CORRECTION  2

// macros used for the basic colors
#define WHITE  cv::Scalar(255, 255, 255)
#define BLACK  cv::Scalar(  0,   0,   0)
#define BLUE   cv::Scalar(255,   0,   0)
#define GREEN  cv::Scalar(  0, 255,   0)
#define RED    cv::Scalar(  0,   0, 255)
#define YELLOW cv::Scalar(255, 255,   0)

// macros used for the field representation 
#define ENTITY_C cv::Scalar(152, 255, 152)
#define AXIS_C   cv::Scalar(220, 220, 220)
#define FIELD_C  cv::Scalar(105, 105, 105)

// macros used for the color detection range
#define LOWER_LIMIT cv::Scalar(60, 100, 100)
#define UPPER_LIMIT cv::Scalar(60, 255, 255)

namespace id
{
	/***************** functions *******************/

	/**
	 * This function is used to display the field data in the debug console
	 *
	 * @param
	 * @param
	 */
	void debug_field_data(int line_count, int col_count);

	/**
	 * This function is used to retrieve the field data from the input manager
	 *
	 */
	void init_field_data();

	/**
	 * This function is used to create the field based on the collected data
	 *
	 * @return
	 */
	cv::Mat init_ui_field();

	/**
	 * This function is used to create the field based on the collected data
	 *
	 * @return
	 */
	cv::Mat get_countours(cv::Mat map, cv::Mat drawing);
}

#endif