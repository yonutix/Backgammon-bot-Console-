#include "def.h"

int main(int argc, char *argv[])
{
	if (argc < 5) {
		printf("Usage: %s server_hostname server_port opponent_level"
		    "(1=dumb, 5, 7, 8)own_level(1=dumb, 5, 7, 8)\n", argv[0]);
		return 0;
	}
    Connection *link = new Connection(argv[1], argv[2]);
	AI *brain = new AI(argv[3], link);
	brain->mainloop();
	return 0;
}
