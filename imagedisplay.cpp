#include "inputmanager.h"
#include "imagedisplay.h"

#include <stdio.h>
#include <string>

/************************* globals **************************/

// window properties
int WIDTH  = 1;
int HEIGHT = 1;
int DEBUG  = DEBUG_OFF;

// entity-related globals
int ENTITY_COUNTER;

cv::Mat OUTPUT(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);

std::vector<ENTITY>                 ENTITIES;
std::vector<std::vector<int>>       FIELD;
std::vector<std::vector<cv::Point>> CONTOURS;

/*********************** functions *************************/

// test for some additional stuff on the gui window
void mouse_callback(int event, int x, int y, int flag, void* param) { if (event == cv::EVENT_MOUSEMOVE) std::cout << x << " " << y << std::endl; }

void display_image()
{
	cv::Mat gui(cv::Size(WIDTH, GUI_HEIGHT), CV_8UC3, FIELD_C);

	while (true)
	{		
		
		/*
		cv::Mat3b gui_3b = gui;
		cv::Mat3b out_3b = OUTPUT;
		// this adds a gui image below the OUTPUT image
		// throws an error because of the channels being different (CV_8UC3 and CV_8UC1)
		int height = HEIGHT + GUI_HEIGHT;
		cv::Mat3b output(WIDTH, height, cv::Vec3b(0,0,0));
		out_3b.copyTo(output(cv::Rect(NULL, NULL, out_3b.cols, out_3b.rows)));
		gui_3b.copyTo(output(cv::Rect(NULL, HEIGHT, gui_3b.cols, gui_3b.rows)));
		*/

		cv::imshow(WINDOW, OUTPUT);
		cv::setMouseCallback(WINDOW, mouse_callback);
		cv::waitKey(1);
	}
}

void debug_field_data()
{
	int line_count = HEIGHT / SIZE_MULTIPLIER;
	int col_count  = WIDTH / SIZE_MULTIPLIER;

	for (size_t i = 0; i < FIELD.size(); i++)
	{
		for (size_t j = 0; j < FIELD[i].size(); j++)
			std::cout << FIELD[i][j];
		std::cout << std::endl;
	}

	std::cout << "number of lines:   " << line_count << std::endl;
	std::cout << "number of columns: " << col_count  << std::endl;
}

int init_field_data(int level, std::string path)
{
	int line_count;
	int col_count;

	// try getting level data
	try {
		std::tie(line_count, col_count, FIELD) = im::get_input_data(level, path);
	}
	catch (std::string e) { std::cout << e << std::endl; return NOT_OK; }

	WIDTH  = col_count  * SIZE_MULTIPLIER;
	HEIGHT = line_count * SIZE_MULTIPLIER;

	return OK;
}

cv::Mat update_ui_field()
{
	cv::Mat target(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);

	for (size_t i = 0; i < FIELD.size(); i++)
		for (size_t j = 0; j < FIELD[i].size(); j++)
		{
			if (FIELD[i][j] == 0)
			{
				int tl_x = (j * SIZE_MULTIPLIER) + 1;
				int tl_y = (i * SIZE_MULTIPLIER) + 1;

				int br_x = (j * SIZE_MULTIPLIER) + (SIZE_MULTIPLIER - 1);
				int br_y = (i * SIZE_MULTIPLIER) + (SIZE_MULTIPLIER - 1);

				cv::rectangle(target, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), FIELD_C, cv::FILLED);
			}

			if (FIELD[i][j] == 1)
			{
				int tl_x = (j * SIZE_MULTIPLIER) + 1;
				int tl_y = (i * SIZE_MULTIPLIER) + 1;

				int br_x = (j * SIZE_MULTIPLIER) + SIZE_MULTIPLIER - 1;
				int br_y = (i * SIZE_MULTIPLIER) + SIZE_MULTIPLIER - 1;

				cv::rectangle(target, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), ENTITY_C, cv::FILLED);
			}
		}

	return target;
}

int preprocess_mat(cv::Mat target, int flag, int debug)
{
	if (!debug) 
	{
		if (flag ==    BLUR) cv::GaussianBlur(target, target, cv::Size(3, 3), 3, 0);
		if (flag ==  TO_HSV) cv::cvtColor(target, target, cv::COLOR_BGR2HSV);
		if (flag == ISOLATE) cv::inRange(target, LOWER_LIMIT, UPPER_LIMIT, target); // ???

		return OK;
	}

	// if debugging is enabled, convert the image based on the flag then send it to OUTPUT
	preprocess_mat(target, flag, DEBUG_OFF);
	OUTPUT = target;
	std::this_thread::sleep_for(std::chrono::seconds(5));

	return OK;
}

int process_mat(cv::Mat target, cv::Mat output)
{	
	OUTPUT = target;
	if (DEBUG) std::this_thread::sleep_for(std::chrono::seconds(5));

	if (!preprocess_mat(target, BLUR,    DEBUG)) std::cout << PREPROCESS_ERROR << ": blur"    << std::endl;
	if (!preprocess_mat(target, TO_HSV,  DEBUG)) std::cout << PREPROCESS_ERROR << ": to_hsv"  << std::endl;
	if (!preprocess_mat(target, ISOLATE, DEBUG)) std::cout << PREPROCESS_ERROR << ": isolate" << std::endl;  // this doesn't. why??

	cv::inRange(target, LOWER_LIMIT, UPPER_LIMIT, target); // this works
	OUTPUT = target;
	// this stays here until i figure out why the cv::inRange method does not work above
	if (DEBUG) std::this_thread::sleep_for(std::chrono::seconds(5));

	// detects contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(target, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>> contours_poly(contours.size());
	std::vector<cv::Rect> boundRect(contours.size());

	// finds the number of entities and their coordinates and saves them in the ENTITY vector
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(contours[i], contours_poly[i], 3, true);
		boundRect[i] = cv::boundingRect(contours_poly[i]);

		int height = int(boundRect[i].height / SIZE_MULTIPLIER + 1);
		int width  = int(boundRect[i].width  / SIZE_MULTIPLIER + 1);

		CONTOURS.push_back(contours[i]);

		if (width == 1 || height == 1)
			continue;

		// applies a correction of 2 to the width and height of the entity with respect to the bounding box
		width  += BOUNDARY_CORRECTION;
		height += BOUNDARY_CORRECTION;

		int x = int(boundRect[i].tl().x / SIZE_MULTIPLIER) - 1;
		int y = int(boundRect[i].tl().y / SIZE_MULTIPLIER) - 1;

		ENTITIES.push_back(ENTITY());
		ENTITIES[ENTITY_COUNTER].body    = std::make_tuple(x, y, width, height);
		ENTITIES[ENTITY_COUNTER].cont_id = int(i);
		ENTITIES[ENTITY_COUNTER].shape   = target(boundRect[i]);

		ENTITY_COUNTER++;
	}

	// draws entity boundaries
	for (size_t i = 0; i < contours.size(); i++)
	{
		int height = int(boundRect[i].height / SIZE_MULTIPLIER) + 1;
		int width  = int(boundRect[i].width  / SIZE_MULTIPLIER) + 1;

		if (width == 1 || height == 1)
			continue;

		// arbitrary values based on personal taste on visuals
		int tl_x = boundRect[i].tl().x - SIZE_MULTIPLIER + 5;
		int tl_y = boundRect[i].tl().y - SIZE_MULTIPLIER + 5;
		int br_x = boundRect[i].br().x + SIZE_MULTIPLIER - 5;
		int br_y = boundRect[i].br().y + SIZE_MULTIPLIER - 5;
		cv::rectangle(output, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), RED, 5);

		int tl_ox = boundRect[i].tl().x - SIZE_MULTIPLIER;
		int tl_oy = boundRect[i].tl().y - SIZE_MULTIPLIER;
		int br_ox = boundRect[i].tl().x - 1;
		int br_oy = boundRect[i].tl().y - 1;
		cv::rectangle(output, cv::Point(tl_ox, tl_oy), cv::Point(br_ox, br_oy), BLUE, cv::FILLED);
	}

	OUTPUT = output;

	return OK;
}

bool coord_comp(ENTITY left, ENTITY right) { return std::get<0>(left.body) < std::get<0>(right.body); }

bool shape_match(const cv::Mat& shape1, const cv::Mat& shape2)
{
	if ((shape1.rows != shape2.rows) || (shape1.cols != shape2.cols))
		return false;
	cv::Scalar s = cv::sum(shape1 - shape2);
	return (s[0] == 0) && (s[1] == 0) && (s[2] == 0);
}

cv::Mat rotate_shape(cv::Mat shape, double angle)
{
	cv::Point2f center((shape.cols - 1) / 2.0, (shape.rows - 1) / 2.0);
	cv::Mat rotation = cv::getRotationMatrix2D(center, angle, 1.0);
	cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), shape.size(), angle).boundingRect2f();

	rotation.at<double>(0, 2) += bbox.width / 2.0  - shape.cols / 2.0;
	rotation.at<double>(1, 2) += bbox.height / 2.0 - shape.rows / 2.0;

	cv::Mat target_shape;
	cv::warpAffine(shape, target_shape, rotation, bbox.size());

	return target_shape;
}

void export_entities_number()
{
	std::cout << "number of non-noise entities: " << ENTITY_COUNTER << std::endl;
}

void export_entities_properties()
{
	for (auto i : ENTITIES)
		printf(FORMAT_BASICS, std::get<0>(i.body), std::get<1>(i.body), std::get<2>(i.body), std::get<3>(i.body));
}

void export_entities_duplicates()
{
	std::vector<int> skippable;
	for (size_t i = 0; i < ENTITIES.size(); i++)
	{
		if (std::find(skippable.begin(), skippable.end(), i) != skippable.end()) continue;

		std::vector<ENTITY> output;
		output.push_back(ENTITIES[i]);

		for (size_t j = 0; j < ENTITIES.size(); j++)
		{
			if (ENTITIES[i].cont_id == ENTITIES[j].cont_id) continue;
			double match = cv::matchShapes(CONTOURS[ENTITIES[i].cont_id], CONTOURS[ENTITIES[j].cont_id], 1, 0.0);

			// while debugging, i found out that some entities have a contour match as following: 0 < match < 1*10^(-7) 
			// because of this, changing the condition from (match == 0) to being lower than a certain threshold solves the problem
			if (match <= 0.00001)
			{
				if (shape_match(ENTITIES[i].shape, ENTITIES[j].shape))
				{
					output.push_back(ENTITIES[j]);
					skippable.push_back(j);

					printf(FORMAT_STD, std::get<0>(ENTITIES[i].body), std::get<1>(ENTITIES[i].body),
						std::get<2>(ENTITIES[i].body), std::get<3>(ENTITIES[i].body));
					printf(FORMAT_DUP, std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));

					continue;
				}
			}
		}

		for (auto i : output)
			if (output.size() == 1)
				printf(FORMAT_BASICS, std::get<0>(i.body), std::get<1>(i.body), std::get<2>(i.body), std::get<3>(i.body));
	}
}

void export_entities_rotated()
{
	std::vector<int> skippable;
	for (size_t i = 0; i < ENTITIES.size(); i++)
	{
		if (std::find(skippable.begin(), skippable.end(), i) != skippable.end()) continue;

		std::vector<ENTITY> output;
		output.push_back(ENTITIES[i]);

		for (size_t j = 0; j < ENTITIES.size(); j++)
		{
			if (ENTITIES[i].cont_id == ENTITIES[j].cont_id) continue;
			double match = cv::matchShapes(CONTOURS[ENTITIES[i].cont_id], CONTOURS[ENTITIES[j].cont_id], 1, 0.0);

			// while debugging, i found out that some entities have a contour match as following: 0 < match < 1*10^(-7) 
			// because of this, changing the condition from (match == 0) to being lower than a certain threshold solves the problem
			if (match <= 0.00001)
			{
				printf(FORMAT_STD, std::get<0>(ENTITIES[i].body), std::get<1>(ENTITIES[i].body),
					std::get<2>(ENTITIES[i].body), std::get<3>(ENTITIES[i].body));
				output.push_back(ENTITIES[j]);
				skippable.push_back(j);
				
				if (shape_match(ENTITIES[i].shape, ENTITIES[j].shape)) 
				{
					printf(FORMAT_DUP, std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
					continue;
				}
				
				// compares if the two shapes are the same, to check if there is any rotation
				if (shape_match(ENTITIES[i].shape, rotate_shape(ENTITIES[j].shape,  90))) 
				{ 
					printf(FORMAT_90D, std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
					continue;
				}
				if (shape_match(ENTITIES[i].shape, rotate_shape(ENTITIES[j].shape, 180))) 
				{
					printf(FORMAT_180D, std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
					continue;
				}
				if (shape_match(ENTITIES[i].shape, rotate_shape(ENTITIES[j].shape, 270))) 
				{
					printf(FORMAT_270D, std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
					continue;
				}
			}
		}

		for (auto i : output)
			if (output.size() == 1)
				printf(FORMAT_BASICS, std::get<0>(i.body), std::get<1>(i.body), std::get<2>(i.body), std::get<3>(i.body));
	}
}

void id_entry_point(int selected_level, cv::Mat target)
{
	OUTPUT = target;

	cv::Mat output = target.clone();
	process_mat(target, output);
	
	std::sort(ENTITIES.begin(), ENTITIES.begin() + ENTITIES.size(), coord_comp);

	if (selected_level == LEVEL_1) export_entities_number();
	if (selected_level == LEVEL_2) export_entities_properties();
	if (selected_level == LEVEL_3) export_entities_duplicates();
	if (selected_level == LEVEL_4) export_entities_rotated();
}

void set_debug(int debug_value) { DEBUG = debug_value; }

void reset_globals()
{
	WIDTH  = 1;
	HEIGHT = 1;
	ENTITY_COUNTER = 0;

	ENTITIES.clear();
	FIELD.clear();
	CONTOURS.clear();
}

