#include "common/types.h"
#include "common/log.h"
#include "streamer.h"

#include <signal.h>

Streamer streamer;

void sighandler(int /*sig*/)
{
	streamer.stop();
	LOGW("Program terminated!");
	exit(1);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	Log::create();
	if (!streamer.start())
		return 1;
	return 0;
}
