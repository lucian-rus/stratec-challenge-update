#include "imagedisplay.h"

int main()
{
	std::thread console(console_command);
	std::thread display(display_image);

	display.join();
	console.join();

	return 0;
}