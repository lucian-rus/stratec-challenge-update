#include "inputmanager.h"
#include "imagedisplay.h"

#include <stdio.h>

/************************* globals **************************/

// window properties
int WIDTH  = 1;
int HEIGHT = 1;

// entity-related globals
int ENTITY_COUNTER;

cv::Mat OUTPUT(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);

std::vector<ENTITY> ENTITIES;

std::vector<std::vector<int>> FIELD;

std::vector<std::vector<cv::Point>> CONTOURS;

/*********************** functions *************************/

void display_image()
{
	while (true)
	{
		cv::imshow("map", OUTPUT);
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
	std::cout << "number of columns: " << col_count << std::endl;
}

int init_field_data(int level)
{
	int line_count;
	int col_count;

	// try getting level data
	try {
		std::tie(line_count, col_count, FIELD) = im::get_input_data(level);
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

// processes the image based on the given flag
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
	preprocess_mat(target, flag, 0);
	OUTPUT = target;
	std::this_thread::sleep_for(std::chrono::seconds(5));

	return OK;
}

// saves the contours
int process_mat(cv::Mat target, cv::Mat output)
{	
	int debug = DEBUG_OFF;

	// process the image based on the flag 
	if (!preprocess_mat(target, BLUR,    debug)) std::cout << PREPROCESS_ERROR << ": blur"    << std::endl;
	if (!preprocess_mat(target, TO_HSV,  debug)) std::cout << PREPROCESS_ERROR << ": to_hsv"  << std::endl;
	if (!preprocess_mat(target, ISOLATE, debug)) std::cout << PREPROCESS_ERROR << ": isolate" << std::endl;  // this doesn't. why??

	cv::inRange(target, LOWER_LIMIT, UPPER_LIMIT, target); // this works
	OUTPUT = target;
	std::this_thread::sleep_for(std::chrono::seconds(5));

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

		ENTITY_COUNTER++;
	}

	// draws entity boundaries
	for (size_t i = 0; i < contours.size(); i++)
	{
		int height = int(boundRect[i].height / SIZE_MULTIPLIER) + 1;
		int width  = int(boundRect[i].width  / SIZE_MULTIPLIER) + 1;

		if (width == 1 || height == 1)
			continue;

		int tl_x = boundRect[i].tl().x - SIZE_MULTIPLIER + 5;
		int tl_y = boundRect[i].tl().y - SIZE_MULTIPLIER + 5;

		int br_x = boundRect[i].br().x + SIZE_MULTIPLIER - 5;
		int br_y = boundRect[i].br().y + SIZE_MULTIPLIER - 5;

		cv::rectangle(output, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), RED, 5);
	}

	OUTPUT = output;

	return OK;
}

bool coord_comp(ENTITY left, ENTITY right) { return std::get<0>(left.body) < std::get<0>(right.body); }

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
	for (size_t i = 0; i < ENTITIES.size(); i++)
		for (size_t j = 0; j < ENTITIES.size(); j++)
		{
			if (ENTITIES[i].cont_id == ENTITIES[j].cont_id) continue;

			double match = cv::matchShapes(CONTOURS[ENTITIES[i].cont_id], CONTOURS[ENTITIES[j].cont_id], 1, 0.0);
			if (match == 0)
			{
				printf(FORMAT_BASICS, std::get<0>(ENTITIES[i].body), std::get<1>(ENTITIES[i].body),
					                  std::get<2>(ENTITIES[i].body), std::get<3>(ENTITIES[i].body));
				printf("also found at (%d, %d)\n", std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
			}

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
}

void reset_globals()
{
	WIDTH  = 1;
	HEIGHT = 1;
	ENTITY_COUNTER = 0;

	OUTPUT = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);
	ENTITIES.clear();
	FIELD.clear();
	CONTOURS.clear();
}

// todo and delete
void console_command()
{
	std::string input;
	std::cout << "> ";
	while (std::cin >> input)
	{
		std::cout << "> ";
		if (input == "start")
		{
			if (!init_field_data(LEVEL_2)) { std::cout << "error trying to import data" << std::endl; break;  }
			debug_field_data();

			cv::Mat window = update_ui_field();
			id_entry_point(LEVEL_2, window);
		}
		if (input == "exit") { exit(EXIT_SUCCESS); }
	}
}