#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "nlohmann/json.hpp"
#include "Networking.h"
#include <fstream>
#include <dirent.h>
#include <vector>
//#include <UI.h>
using namespace std;
using json = nlohmann::json;

class AmiiboVars
{
	public:
	string AmiiboSeries = "";
	string AmiiboName = "";
	int ListIndex = 0;
};

class CreatorUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawAmiiboList();
	void DrawSeriesList();
	void DrawFooter();
	int HeaderHeight;
	int FooterHeight;
	int ListHeight;
	int TouchX = -1;
	int TouchY = -1;
	json JData;
	int JDataSize = 0;
	bool HasSelectedSeries = false;
	vector<string> SeriesVec;
	vector<AmiiboVars> AmiiboVarsVec;
	vector<AmiiboVars> SortedAmiiboVarsVec;
	string AmiiboAPIString = "";
	void PleaseWait(string mensage);
	void Createlist();
	SDL_Surface* PrevIcon;//surface buffer to amiibo select image
	bool Creatype = true;
	int Ordetype = 1;
	int DownPrev = 0;
	int imgres = 20;
	int indexb1 = 0;
	int indexb2 = 0;
	int indexb3 = 0;
	string List = "amiiboSeries";

	public:
	CreatorUI();
	void GetInput();
	void DrawUI();
	void GetDataFromAPI();
	void InitList();
	void ListSelect();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
	ScrollList *SeriesList;
	ScrollList *MenuList;
	int SeriesListWidth;
	string *CurrentPath;
};

