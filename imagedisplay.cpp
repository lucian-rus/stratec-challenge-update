#include "inputmanager.h"
#include "imagedisplay.h"

#include <stdio.h>

#define ENTITY std::vector<std::tuple<int, int, int, int>>

/************************* globals **************************/

int WIDTH;
int HEIGHT;
int ENTITY_COUNTER;

std::vector<std::vector<int>> FIELD;

/**
 * This vector contains the entities found in the field
 * <x, y, width, height>
 */
ENTITY ENTITIES;


std::vector<std::vector<cv::Point>> CONTOURS;

/*********************** functions *************************/

void debug_field_data(int line_count, int col_count)
{
	for (int i = 0; i < FIELD.size(); i++)
	{
		for (int j = 0; j < FIELD[i].size(); j++)
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
		std::tie(line_count, col_count, FIELD) = im::get_input_data(LEVEL_BASICS);
	}
	catch (std::string e) { std::cout << e << std::endl; }

	debug_field_data(line_count, col_count);

	WIDTH = col_count * SIZE_MULTIPLIER;
	HEIGHT = line_count * SIZE_MULTIPLIER;
}

cv::Mat init_ui_field()
{
	cv::Mat window(cv::Size(WIDTH, HEIGHT), CV_8UC3, BLACK);

	for (int i = 0; i < FIELD.size(); i++)
		for (int j = 0; j < FIELD[i].size(); j++)
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

		// debug purposes
		std::cout << "entity " << i << ": " << width << " " << height << std::endl;

		if (width == 1 || height == 1)
			continue;

		CONTOURS.push_back(contours[i]);

		// applies a correction of 2 to the width and height of the entity with respect to the bounding box
		width += BOUNDARY_CORRECTION;
		height += BOUNDARY_CORRECTION;

		int x = int(boundRect[i].tl().x / SIZE_MULTIPLIER) - 1;
		int y = int(boundRect[i].tl().y / SIZE_MULTIPLIER) - 1;

		ENTITIES.push_back(std::make_tuple(x, y, width, height));

		entity_counter++;
	}

	std::cout << "number of entities: " << entity_counter << std::endl;

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
bool coord_comp(std::tuple<int, int, int, int> left, std::tuple<int, int, int, int> right) { return std::get<0>(left) < std::get<0>(right); }

int main()
{
	init_field_data();
	cv::Mat ui_map = init_ui_field();
	cv::imshow("map", ui_map);

	cv::Mat output = ui_map.clone();
	get_countours(ui_map, output);

	// sort the vector
	std::sort(ENTITIES.begin(), ENTITIES.begin() + ENTITIES.size(), coord_comp);
	for (auto i : ENTITIES)
		printf("(%d, %d) W: %d, H: %d\n", std::get<0>(i), std::get<1>(i), std::get<2>(i), std::get<3>(i));


	// compare the contours of the entities (0 if identical)
	for (size_t i = 0; i < CONTOURS.size(); i++)
		for (size_t j = 0; j < CONTOURS.size(); j++)
		{
			if (i == j) continue;
			double match = cv::matchShapes(CONTOURS[i], CONTOURS[j], 1, 0.0);
			printf("%d matches %d by %d\n", i, j, match);
		}

	return 0;
}