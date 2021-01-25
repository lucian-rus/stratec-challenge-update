#include "imagedisplay.h"
#include "inputmanager.h"

// todo and delete
// this needs an update, very NOT elegant approach to CLI implementation
void console_command()
{
	std::string input;
	std::cout << "> ";
	while (std::cin >> input)
	{
		if (input.find("level") != std::string::npos)
		{
			int map, level;
			if (input == "level1") { map = MAP_1; level = LEVEL_1; }
			if (input == "level2") { map = MAP_1; level = LEVEL_2; }
			if (input == "level3") { map = MAP_2; level = LEVEL_3; }
			if (input == "level4") { map = MAP_3; level = LEVEL_4; }

			if (!init_field_data(map)) { std::cout << INVALID_PATH_CALL << std::endl; break; }
			id_entry_point(level, update_ui_field());
			reset_globals();

			std::cout << "> ";
			continue;
		}
		
		if (input == "debugon")  { set_debug(DEBUG_ON);  std::cout << "> "; continue; }
		if (input == "debugoff") { set_debug(DEBUG_OFF); std::cout << "> "; continue; }

		if (input == "exit") { exit(EXIT_SUCCESS); }

		std::cout << "no suitable command" << std::endl;
		std::cout << "> ";
	}
}

int main()
{
	std::thread console(console_command);
	std::thread display(display_image);

	display.join();
	console.join();

	return 0;
}