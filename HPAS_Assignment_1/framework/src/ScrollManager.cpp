#include "ScrollManager.h"

namespace ScrollManager {
	namespace {
		double scroll = 0.0;
	}

	void onScroll(double scroll)
	{
		ScrollManager::scroll += scroll;
	}

	double getScroll()
	{
        double savedScroll = ScrollManager::scroll;
        ScrollManager::scroll = 0.0;
        return savedScroll;
	}
};