#ifndef LOADSTATEH
#define LOADSTATEH

namespace TilesEditor
{
	enum LoadState {
		STATE_NOT_LOADED,
		STATE_LOADING,
		STATE_LOADED,
		STATE_FAILED
	};
};
#endif