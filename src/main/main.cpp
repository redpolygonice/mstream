#include "common/types.h"
#include "common/log.h"
#include "streamer.h"

#include <signal.h>

void sighandler(int /*sig*/)
{
	GetStreamer()->stop();
	LOGW("Program terminated!");
	exit(1);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	Log::create();
	if (!GetStreamer()->start())
		return 1;
	return 0;
}
