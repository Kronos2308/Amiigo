#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <UI.h>
#include "nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;

class AmiigoUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawFooter();
	int HeaderHeight;
	int FooterHeight;
	vector <dirent> Files{vector <dirent>(0)};
	int TouchX = -1;
	int TouchY = -1;
	ScrollList *AmiiboList;
	json JData;
	SDL_Surface* ActAmiibo;//surface buffer to amiibo image
	SDL_Surface* SelAmiibo;//surface buffer to amiibo sel image
	int ImgAct = 1; //load image triger
	int ImgSel = 1; //load sel image triger
	char CurrentAmiibo[FS_MAX_PATH] = {0};//active amiibo
	public:
	AmiigoUI();
	void GetInput();
	void DrawUI();
	void ScanForAmiibos();
	void PleaseWait(string mensage);
	void InitList();
	void SetAmiibo(int);
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
	ScrollList *MenuList;
	int AmiiboListWidth;
	string ListDir = "sdmc:/emuiibo/amiibo/";
};