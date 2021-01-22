#include "inputmanager.h"
#include "imagedisplay.h"

#include <stdio.h>

/************************* globals **************************/

int WIDTH;
int HEIGHT;
int ENTITY_COUNTER;

std::vector<ENTITY> ENTITIES;

std::vector<std::vector<int>> FIELD;

std::vector<std::vector<cv::Point>> CONTOURS;



/*********************** functions *************************/

void debug_field_data(int line_count, int col_count)
{
	for (size_t i = 0; i < FIELD.size(); i++)
	{
		for (size_t j = 0; j < FIELD[i].size(); j++)
			std::cout << FIELD[i][j];
		std::cout << std::endl;
	}

	std::cout << "number of lines:   " << line_count << std::endl;
	std::cout << "number of columns: " << col_count << std::endl;
}

void init_field_data()
{
	int line_count;
	int col_count;

	// try getting level data
	try {
		std::tie(line_count, col_count, FIELD) = im::get_input_data(LEVEL_DUPLICATES);
	}
	catch (std::string e) { std::cout << e << std::endl; }

	debug_field_data(line_count, col_count);

	WIDTH = col_count * SIZE_MULTIPLIER;
	HEIGHT = line_count * SIZE_MULTIPLIER;
}

cv::Mat init_ui_field()
{
	cv::Mat window(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);

	for (size_t i = 0; i < FIELD.size(); i++)
		for (size_t j = 0; j < FIELD[i].size(); j++)
		{
			if (FIELD[i][j] == 0)
			{
				int tl_x = (j * SIZE_MULTIPLIER) + 1;
				int tl_y = (i * SIZE_MULTIPLIER) + 1;

				int br_x = (j * SIZE_MULTIPLIER) + (SIZE_MULTIPLIER - 1);
				int br_y = (i * SIZE_MULTIPLIER) + (SIZE_MULTIPLIER - 1);

				cv::rectangle(window, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), FIELD_C, cv::FILLED);
			}

			if (FIELD[i][j] == 1)
			{
				int tl_x = (j * SIZE_MULTIPLIER) + 1;
				int tl_y = (i * SIZE_MULTIPLIER) + 1;

				int br_x = (j * SIZE_MULTIPLIER) + SIZE_MULTIPLIER - 1;
				int br_y = (i * SIZE_MULTIPLIER) + SIZE_MULTIPLIER - 1;

				cv::rectangle(window, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), ENTITY_C, cv::FILLED);
			}
		}

	return window;
}

cv::Mat get_countours(cv::Mat map, cv::Mat drawing)
{
	// process image 
	cv::GaussianBlur(map, map, cv::Size(3, 3), 3, 0);
	cv::cvtColor(map, map, cv::COLOR_BGR2HSV);
	cv::inRange(map, LOWER_LIMIT, UPPER_LIMIT, map);

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(map, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point>> contours_poly(contours.size());
	std::vector<cv::Rect> boundRect(contours.size());

	int entity_counter = 0;

	// finds the number of entities and their coordinates and saves them in the ENTITY vector
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(contours[i], contours_poly[i], 3, true);
		boundRect[i] = cv::boundingRect(contours_poly[i]);

		int height = int(boundRect[i].height / SIZE_MULTIPLIER + 1);
		int width = int(boundRect[i].width / SIZE_MULTIPLIER + 1);

		//debug purposes
		//std::cout << "entity " << i << ": " << width << " " << height << std::endl;

		CONTOURS.push_back(contours[i]);

		if (width == 1 || height == 1)
			continue;

		// applies a correction of 2 to the width and height of the entity with respect to the bounding box
		width  += BOUNDARY_CORRECTION;
		height += BOUNDARY_CORRECTION;

		int x = int(boundRect[i].tl().x / SIZE_MULTIPLIER) - 1;
		int y = int(boundRect[i].tl().y / SIZE_MULTIPLIER) - 1;

		ENTITIES.push_back(ENTITY());
		ENTITIES[entity_counter].cont_id = i;
		ENTITIES[entity_counter].body    = std::make_tuple(x, y, width, height);

		entity_counter++;
	}

	ENTITY_COUNTER = entity_counter;

	// draws entity boundaries
	for (size_t i = 0; i < contours.size(); i++)
	{
		int height = int(boundRect[i].height / SIZE_MULTIPLIER) + 1;
		int width = int(boundRect[i].width / SIZE_MULTIPLIER) + 1;

		if (width == 1 || height == 1)
			continue;

		int tl_x = boundRect[i].tl().x - SIZE_MULTIPLIER + 5;
		int tl_y = boundRect[i].tl().y - SIZE_MULTIPLIER + 5;

		int br_x = boundRect[i].br().x + SIZE_MULTIPLIER - 5;
		int br_y = boundRect[i].br().y + SIZE_MULTIPLIER - 5;

		cv::rectangle(drawing, cv::Point(tl_x, tl_y), cv::Point(br_x, br_y), RED, 5);
	}

	cv::imshow("output", drawing);
	cv::waitKey(0);
	return map;
}

// compares the first value of two given tuples
bool coord_comp(ENTITY left, ENTITY right) { return std::get<0>(left.body) < std::get<0>(right.body); }

int main()
{
	init_field_data();
	cv::Mat ui_map = init_ui_field();
	cv::imshow("map", ui_map);

	cv::Mat output = ui_map.clone();
	get_countours(ui_map, output);

	// solves level 1
	std::cout << "number of non-noise entities: " << ENTITY_COUNTER << std::endl;

	// solves level 2
	std::sort(ENTITIES.begin(), ENTITIES.begin() + ENTITIES.size(), coord_comp);
	for (auto i : ENTITIES)
		printf("(%d, %d) W: %d, H: %d\n", std::get<0>(i.body), std::get<1>(i.body), std::get<2>(i.body), std::get<3>(i.body));
	
	// solves level 3
	for (size_t i = 0; i < ENTITIES.size(); i++) 
		for (size_t j = 0; j < ENTITIES.size(); j++)
		{
			if (ENTITIES[i].cont_id == ENTITIES[j].cont_id) continue;

			double match = cv::matchShapes(CONTOURS[ENTITIES[i].cont_id], CONTOURS[ENTITIES[j].cont_id], 1, 0.0);
			if (match == 0)
			{
				printf("(%d, %d) W: %d, H: %d ",   std::get<0>(ENTITIES[i].body), std::get<1>(ENTITIES[i].body),
												   std::get<2>(ENTITIES[i].body), std::get<3>(ENTITIES[i].body));
				printf("also found at (%d, %d)\n", std::get<0>(ENTITIES[j].body), std::get<1>(ENTITIES[j].body));
			}
			
		}
	 
	return 0;
}