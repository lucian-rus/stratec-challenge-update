#pragma once
#ifndef INPUTMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <tuple>

/******************* macros *********************/

#define NULL 0

// macro representing the desired level
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3

// macro representing the path to the desired input file
#define LEVEL_1_PATH "resources/csv/basics.csv"
#define LEVEL_2_PATH "resources/csv/duplicates.csv"
#define LEVEL_3_PATH "resources/csv/advanced.csv"

// macro representing the error that has been encountered
#define FILE_OPEN_ERROR   "error opening the requested .csv file"
#define INVALID_PATH_CALL "no file found based on the provided path"
#define INVALID_ID_CALL   "invalid level id call"

namespace im
{
	/***************** functions *******************/

	/**
	 * This function casts a type `char` variable to type `int`
	 *
	 * @param
	 */
	int ctoi(char c);

	/**
	 * This function returns a 2D vector that contains the input field
	 *
	 * @param
	 */
	std::vector<std::vector<int>> get_file_input(std::string path);

	/**
	 * This function returns the data that was collected from the input file.
	 *
	 * @param level_id Integer representing the level from which the data is collected
	 */
	std::tuple<int, int, std::vector<std::vector<int>>> get_input_data(int level_id);
}

#endif 
