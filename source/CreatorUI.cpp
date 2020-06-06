//Note this is extremely janky. I made it while tired.
//I should have probably made a common UI class for this and AmiigoUI but I didn't so there's a lot of reused code.
//It could definitely be more efficient. We only really need to call GetDataFromAPI() once but I'm lazy so I didn't.
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include "nlohmann/json.hpp"
#include "Networking.h"
#include <fstream>
#include <dirent.h>
#include <vector>
#include <UI.h>
#include "Utils.h"
using namespace std;
using json = nlohmann::json;

SDL_Surface* CIcon;//surface buffer to amiibo sel image
int Ordetype = 1;
int Creatype = 0;
int DownPrev = 0;
int imgres = 20;

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
	public:
	CreatorUI();
	void GetInput();
	void DrawUI();
	void GetDataFromAPI(string);
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

CreatorUI::CreatorUI()
{
	nifmInitialize(NifmServiceType_User); //Init nifm for connection stuff
	HeaderFont = GetSharedFont(48);
	ListFont = GetSharedFont(32);
	//HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48); //Load the header font
	//ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
	GetDataFromAPI(""); //Get data from the API
	
	//Create the lists
	SeriesList = new ScrollList();
	Createlist();
}

void CreatorUI::Createlist()
{
	AmiiboVarsVec.clear();
	SeriesVec.clear();
	string List = "amiiboSeries";
	switch(Ordetype)
	{
		case 1:
		List = "amiiboSeries";
		break;
		
		case 2:
		List = "character";
		break;
		
		case 3:
		List = "gameSeries";
		break;
	}	
	//Get all of the Series' names and add Amiibos to the AmiiboVarsVec
	for(int i = 0; i < JDataSize; i++)
	{
		bool IsInVec = false;
		string SeriesName = JData["amiibo"][i][List].get<std::string>();
		
		//Add data to the AmiiboVarsVec
		AmiiboVars TempAmiiboVars;
		TempAmiiboVars.AmiiboSeries = SeriesName;
		TempAmiiboVars.AmiiboName = JData["amiibo"][i]["name"].get<std::string>();
		TempAmiiboVars.ListIndex = i;
		AmiiboVarsVec.push_back(TempAmiiboVars);
		
		//Loop through every element in the vector
		for(int j = 0; j < (int)SeriesVec.size(); j++)
		{
			//If the vector has the name we break the loop
			if(SeriesVec.at(j) == SeriesName)
			{
				IsInVec = true;
				break;
			}
		}
		if(!IsInVec)
		{
			SeriesVec.push_back(JData["amiibo"][i][List].get<std::string>());
		}
	}
	
}

void CreatorUI::InitList()
{
	//Create the lists
	SeriesList->TouchListX = &TouchX;
	SeriesList->TouchListY = &TouchY;
	SeriesList->ListFont = GetSharedFont(32); //Load the list font
	SeriesList->ListingsOnScreen = 10;
	SeriesList->ListWidth = SeriesListWidth;
	SeriesList->renderer = renderer;
	SeriesList->ListingTextVec = SeriesVec;
}

void CreatorUI::GetInput()
{
	//Scan input
	while (SDL_PollEvent(Event))
		{
			//printf("Button-ID-%d-\n",Event->jbutton.button);
            switch (Event->type)
			{
				//Touchscreen
				case SDL_FINGERDOWN:
				TouchX = Event->tfinger.x * *Width;
				TouchY = Event->tfinger.y * *Height;
				//Set the touch list pointers because we need them to work in both menus
				MenuList->TouchListX = &TouchX;
				MenuList->TouchListY = &TouchY;
				break;
				//Joycon button pressed
                case SDL_JOYBUTTONDOWN:
                    if (Event->jbutton.which == 0)
					{
						//Plus pressed
						if (Event->jbutton.button == 10)
						{
                            *IsDone = 1;
                        }
						//L pressed preview
						else if(Event->jbutton.button == 7)
						{
							if(SeriesList->IsActive&HasSelectedSeries) {DownPrev = 1;}
						}
						//Up pressed
						else if(Event->jbutton.button == 13||Event->jbutton.button == 17)
						{
							if(SeriesList->IsActive)
							{
								SeriesList->CursorIndex--;
								SeriesList->SelectedIndex--;
							}
							else
							{
								MenuList->CursorIndex--;
								MenuList->SelectedIndex--;
							}
						}
						//Down pressed
						else if(Event->jbutton.button == 15||Event->jbutton.button == 19)
						{
							if(SeriesList->IsActive)
							{
								SeriesList->CursorIndex++;
								SeriesList->SelectedIndex++;
							}
							else
							{
								MenuList->CursorIndex++;
								MenuList->SelectedIndex++;
							}
						}
						//Left or right pressed
						else if(Event->jbutton.button == 12 || Event->jbutton.button == 14|| Event->jbutton.button == 16|| Event->jbutton.button == 18)
						{
							MenuList->IsActive = SeriesList->IsActive;
							SeriesList->IsActive = !SeriesList->IsActive;
						}
						//A pressed
						else if(Event->jbutton.button == 0)
						{
							if(SeriesList->IsActive)
							{
								ListSelect();
							}
							else
							{
								*WindowState = MenuList->SelectedIndex;
							}
						}
						//B pressed
						else if(Event->jbutton.button == 1)
						{
							//Reset some vars so we don't crash
							SeriesList->ListingTextVec = SeriesVec;
							SeriesList->SelectedIndex = 0;
							SeriesList->CursorIndex = 0;
							SeriesList->ListRenderOffset = 0;
							HasSelectedSeries = false;
						}else if(Event->jbutton.button == 11)
						{
							if(HasSelectedSeries){Creatype++; if(Creatype > 1) {Creatype = 0;}}
							else
							{Ordetype++; if(Ordetype > 3) {Ordetype = 1;} CreatorUI::Createlist();CreatorUI::InitList();}
						}

                    }
                    break;
            }
        }
}

void CreatorUI::DrawUI()
{
	//This crashes when in the constructor for some reason
	HeaderHeight = (*Height / 100) * 10;
	FooterHeight = (*Height / 100) * 10;
	ListHeight = *Height - HeaderHeight;
	SeriesList->ListHeight = *Height - HeaderHeight - FooterHeight;
	SeriesList->ListYOffset = HeaderHeight;
		
	//Draw the BG
	DrawJsonColorConfig(renderer, "CreatorUI_DrawUI");
	SDL_Rect BGRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &BGRect);
	
	//Draw the UI
	DrawHeader();
	SeriesList->DrawList();
	MenuList->DrawList();
	DrawFooter();
	
	if(HasSelectedSeries){
		//Draw box
		DrawJsonColorConfig(renderer, "UI_borders_list");
		SDL_Rect HeaderRect = {690,73, 270, 286};
		SDL_RenderFillRect(renderer, &HeaderRect);
	}

	//preview draw                            //bound check
	if(HasSelectedSeries&SeriesList->IsActive&(SeriesList->SelectedIndex < (int)SortedAmiiboVarsVec.size())){
		int IndexInJdata = SortedAmiiboVarsVec.at(SeriesList->SelectedIndex).ListIndex;
		string ImgPath = "sdmc:/config/amiigo/IMG/"+JData["amiibo"][IndexInJdata]["image"].get<std::string>();
		//download preview by user input
		if (DownPrev){
			if(!CheckFileExists(ImgPath)&HasConnection()){
			PleaseWait("Please wait, Downloading...");
				string icontemp = ImgPath+".temp";
				RetrieveToFile(JData["url"].get<std::string>()+JData["amiibo"][IndexInJdata]["image"].get<std::string>(), icontemp);
				if (fsize(icontemp) != 0) rename(icontemp.c_str(), ImgPath.c_str());
			}
			imgres++;
			DownPrev = 0;
		}
		//load prev img	
		if (imgres != SeriesList->SelectedIndex)
		{
			imgres = SeriesList->SelectedIndex;
			if(CheckFileExists(ImgPath)&(fsize(ImgPath) != 0)) CIcon = IMG_Load(ImgPath.c_str()); else CIcon = IMG_Load("romfs:/download.png");	
			printf("%s\n",ImgPath.c_str());
		} 

		//draw select amiibo image
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		SDL_Texture* Headericon2 = SDL_CreateTextureFromSurface(renderer, CIcon);
		int XM = 695,YM = 75, WM = 260, HM = 280,
		WS = (CIcon->w * (HM * 1000 /CIcon->h) /1000),HS = (CIcon->h * (HM * 1000 /CIcon->w) /1000),
		WT = WS > WM ? WM : WS,HT = WS > WM ? HS : HM,
		XT = XM + (WS < WM ? (WM - WS)/2 : 0),YT = YM + (WS > WM ? (HM - HS) : 0);// printf("print size: %dx%d\n",WS,HM);
		SDL_Rect ImagetRect2 = {XT, YT, WT, HT};
		SDL_RenderCopy(renderer, Headericon2 , NULL, &ImagetRect2);
		SDL_DestroyTexture(Headericon2);
	} else imgres = 20; //reload image if go back

	DrawButtonBorders(renderer, SeriesList, MenuList, HeaderHeight, FooterHeight, *Width, *Height, true);
	//Check if list item selected via touch screen
	if(SeriesList->ItemSelected)
	{
		MenuList->IsActive = false;
		SeriesList->IsActive = true;
	}
	else if(MenuList->ItemSelected)
	{
		*WindowState = MenuList->SelectedIndex;
		MenuList->IsActive = true;
		SeriesList->IsActive = false;
	}
	
	//Reset touch coords
	TouchX = -1;
	TouchY = -1;
}

void CreatorUI::ListSelect()
{
	//Create the virtual amiibo on the SD card
	if(HasSelectedSeries)
	{
		int IndexInJdata = SortedAmiiboVarsVec.at(SeriesList->SelectedIndex).ListIndex;
        string AmiiboPath = *CurrentPath ;
		switch(Creatype)
		{
			case 0:
			AmiiboPath += JData["amiibo"][IndexInJdata]["name"].get<std::string>(); 
			break;
			
			case 1:
				string List = "amiiboSeries";
				switch(Ordetype)
				{
					case 1:
					List = "amiiboSeries";
					break;
					
					case 2:
					List = "character";
					break;
					
					case 3:
					List = "gameSeries";
					break;
				}	

			AmiiboPath = "sdmc:/emuiibo/amiibo/";//force root if you are not on root
			AmiiboPath += JData["amiibo"][IndexInJdata][List].get<std::string>()+"_";
			mkdir(AmiiboPath.c_str(), 0);
			AmiiboPath += "/"+ JData["amiibo"][IndexInJdata]["name"].get<std::string>();
			break;
		}	

 		PleaseWait("Building: "+AmiiboPath.substr(20)+"...");//
		mkdir(AmiiboPath.c_str(), 0);
		
        //Write amiibo.json
        string FilePath = AmiiboPath + "/amiibo.json";
        ofstream CommonFileWriter(FilePath.c_str());
        CommonFileWriter << "{\"first_write_date\": { \"d\": 1, \"m\": 1, \"y\": 2019 }, \"id\": {\"game_character_id\": "+std::to_string(JData["amiibo"][IndexInJdata]["emuiibo"]["game_character_id"].get<int>())+", \"character_variant\": "+std::to_string(JData["amiibo"][IndexInJdata]["emuiibo"]["character_variant"].get<int>())+", \"figure_type\": "+std::to_string(JData["amiibo"][IndexInJdata]["emuiibo"]["figure_type"].get<int>())+",  \"model_number\": "+std::to_string(JData["amiibo"][IndexInJdata]["emuiibo"]["model_number"].get<int>())+", \"series\": "+std::to_string(JData["amiibo"][IndexInJdata]["emuiibo"]["series"].get<int>())+" }, \"last_write_date\": { \"d\": 1, \"m\": 1, \"y\": 2019 }, \"mii_charinfo_file\": \"mii-charinfo.bin\", \"name\": \"" + JData["amiibo"][IndexInJdata]["name"].get<std::string>() + "\", \"version\": 0, \"write_counter\": 0 , \"uuid\": ["+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", "+std::to_string(rand() % 250 + 1)+", 0, 0, 0] }";
        CommonFileWriter.close();
		
        //Write amiibo.flag
        FilePath = AmiiboPath + "/amiibo.flag";
        ofstream ModelFileWriter(FilePath.c_str());
        ModelFileWriter << "";
        ModelFileWriter.close();
		
		//create icon vars
		string iconname = AmiiboPath+"/amiibo.png";
		string icontemp = AmiiboPath+"/amiibo.temp";
		string iconDBex = "sdmc:/config/amiigo/IMG/"+JData["amiibo"][IndexInJdata]["image"].get<std::string>();
		//if exist local used from there
		if(!CheckFileExists(iconname)&CheckFileExists(iconDBex)&(fsize(iconDBex) != 0)) copy_me(iconDBex, iconname);
		
		//get the icon from online
		if(!CheckFileExists(iconname)&HasConnection()){
			RetrieveToFile(JData["url"].get<std::string>()+JData["amiibo"][IndexInJdata]["image"].get<std::string>(), icontemp);
			if (fsize(icontemp) != 0){
			copy_me(icontemp, iconDBex);
			rename(icontemp.c_str(), iconname.c_str());	
			} 
		}
		imgres++;//refresh signal for preview
	}
	//Add the Amiibos from the selected series to the list
	else
	{
		HasSelectedSeries = true;
		string SelectedSeries = SeriesVec.at(SeriesList->SelectedIndex);
		SeriesList->ListingTextVec.clear();
		SortedAmiiboVarsVec.clear();
		for(int i = 0; i < (int)AmiiboVarsVec.size(); i++)
		{
			//There's something happening here
			//What it is ain't exactly clear
			//There's a class with a bug over there
			//When using it we should beware
			if(AmiiboVarsVec.at(i).AmiiboSeries == SelectedSeries)
			{
				SortedAmiiboVarsVec.push_back(AmiiboVarsVec.at(i));
					SeriesList->ListingTextVec.push_back(AmiiboVarsVec.at(i).AmiiboName);

				//SeriesList->ListingTextVec.push_back(SortedAmiiboVarsVec.at(SortedAmiiboVarsVec.size()-1).AmiiboName);
			}
		}
		//Reset some vars so we don't crash
		SeriesList->SelectedIndex = 0;
		SeriesList->CursorIndex = 0;
		SeriesList->ListRenderOffset = 0;
	}
}

void CreatorUI::DrawHeader()
{
	//Draw the header
	DrawJsonColorConfig(renderer, "CreatorUI_DrawHeader");
	SDL_Rect HeaderRect = {0,0, *Width, HeaderHeight};
	SDL_RenderFillRect(renderer, &HeaderRect);

	//draw logo image
	static SDL_Surface* Alogo = IMG_Load("romfs:/icon_large.png");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_Texture* Headericon = SDL_CreateTextureFromSurface(renderer, Alogo);
	SDL_Rect ImagetRect = {1000, 0 , 260, 70};
	SDL_RenderCopy(renderer, Headericon , NULL, &ImagetRect);
	SDL_DestroyTexture(Headericon);

//status text to build amiibos
std::string StatusText = "";
if(HasSelectedSeries){
	if(CheckButtonPressed(&HeaderRect, TouchX, TouchY))
	{
		Creatype++; if(Creatype > 1) {Creatype = 0;}
	}

	switch(Creatype)
	{
		case 0:
		StatusText = "(amiiboName)";
		break;
		
		case 1:
				switch(Ordetype)
				{
					case 1:
					StatusText = "amiiboSeries";
					break;
					
					case 2:
					StatusText = "character";
					break;
					
					case 3:
					StatusText = "gameSeries";
					break;
				}	
		StatusText = "("+StatusText+")";
		break;
	}	
} else {
	if(CheckButtonPressed(&HeaderRect, TouchX, TouchY))
	{
		Ordetype++; if(Ordetype > 3) {Ordetype = 1;} CreatorUI::Createlist();CreatorUI::InitList();
	}

	switch(Ordetype)
	{
		
		case 1:
		StatusText = ">amiiboSeries<";
		break;
		
		case 2:
		StatusText = ">character<";
		break;
		
		case 3:
		StatusText = ">gameSeries<";
		break;
	}	

}


	string headertext = "Amiigo Maker "+StatusText;
	//Draw the Amiibo path text
	SDL_Surface* HeaderTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, headertext.c_str(), TextColour, *Width);
	SDL_Texture* HeaderTextTexture = SDL_CreateTextureFromSurface(renderer, HeaderTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - HeaderTextSurface->w) / 2, (HeaderHeight - HeaderTextSurface->h) / 2, HeaderTextSurface->w, HeaderTextSurface->h};
	SDL_RenderCopy(renderer, HeaderTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(HeaderTextTexture);
	SDL_FreeSurface(HeaderTextSurface);
}

void CreatorUI::GetDataFromAPI(string FilterTerm)
{
		for(int i = 0;i < 3;i++)//wait for the the api
		{
		ifstream DataFileReader("sdmc:/config/amiigo/API.json");
		getline(DataFileReader, AmiiboAPIString);
		DataFileReader.close();		
		if(AmiiboAPIString.size()!=0) break;			
		}
	if(json::accept(AmiiboAPIString))
	{
	//Parse and use the JSON data
	JData = json::parse(AmiiboAPIString);
	JDataSize = JData["amiibo"].size();
			
	}
}

void CreatorUI::DrawFooter()
{
	//Draw the select footer button
	int FooterYOffset = *Height - FooterHeight;
	SDL_Rect SelectFooterRect = {0,FooterYOffset, *Width/2, FooterHeight};
	string FooterText = "Select";
	DrawJsonColorConfig(renderer, "CreatorUI_DrawFooter_Select");
	
	//Select was pressed
	if(CheckButtonPressed(&SelectFooterRect, TouchX, TouchY))
	{
		if(SeriesList->IsActive)
		{
			ListSelect();
		}
		else
		{
			*WindowState = MenuList->SelectedIndex;
		}
	}
	
	SDL_RenderFillRect(renderer, &SelectFooterRect);
	
	//Draw the text
	SDL_Surface* SelectTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, FooterText.c_str(), TextColour, SelectFooterRect.w);
	SDL_Texture* SelectTextTexture = SDL_CreateTextureFromSurface(renderer, SelectTextSurface);
	SDL_Rect FooterTextRect = {(SelectFooterRect.w - SelectTextSurface->w) / 2, FooterYOffset + ((FooterHeight - SelectTextSurface->h) / 2), SelectTextSurface->w, SelectTextSurface->h};
	SDL_RenderCopy(renderer, SelectTextTexture, NULL, &FooterTextRect);
	//Clean up
	SDL_DestroyTexture(SelectTextTexture);
	SDL_FreeSurface(SelectTextSurface);
	
	//Draw the back footer button
	SDL_Rect BackFooterRect = {*Width/2,FooterYOffset, *Width/2, FooterHeight};
	FooterText = "Back";
	DrawJsonColorConfig(renderer, "CreatorUI_DrawFooter_Back");
	
	//Back was pressed
	if(CheckButtonPressed(&BackFooterRect, TouchX, TouchY))
	{
		//Reset some vars so we don't crash
		SeriesList->ListingTextVec = SeriesVec;
		SeriesList->SelectedIndex = 0;
		SeriesList->CursorIndex = 0;
		SeriesList->ListRenderOffset = 0;
		HasSelectedSeries = false;
	}
	
	SDL_RenderFillRect(renderer, &BackFooterRect);
	
	//Draw the status text
	SDL_Surface* BackTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, FooterText.c_str(), TextColour, BackFooterRect.w);
	SDL_Texture* BackTextTexture = SDL_CreateTextureFromSurface(renderer, BackTextSurface);
	SDL_Rect BackTextRect = {BackFooterRect.x + (BackFooterRect.w - BackTextSurface->w) / 2, FooterYOffset + ((FooterHeight - BackTextSurface->h) / 2), BackTextSurface->w, BackTextSurface->h};
	SDL_RenderCopy(renderer, BackTextTexture, NULL, &BackTextRect);
	//Clean up
	SDL_DestroyTexture(BackTextTexture);
	SDL_FreeSurface(BackTextSurface);
}

void CreatorUI::PleaseWait(string mensage)
{
	SDL_Surface* MessageTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, mensage.c_str(), TextColour, *Width);
	//Draw the rect
	DrawJsonColorConfig(renderer, "CreatorUI_PleaseWait");
	SDL_Rect MessageRect = {((*Width - MessageTextSurface->w) / 2)-3,((*Height - MessageTextSurface->h) / 2)-3, (MessageTextSurface->w)+3, (MessageTextSurface->h)+3};
	SDL_RenderFillRect(renderer, &MessageRect);

	//Draw the please wait text
	SDL_Texture* MessagerTextTexture = SDL_CreateTextureFromSurface(renderer, MessageTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - MessageTextSurface->w) / 2, (*Height - MessageTextSurface->h) / 2, MessageTextSurface->w, MessageTextSurface->h};
	SDL_RenderCopy(renderer, MessagerTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(MessagerTextTexture);
	SDL_FreeSurface(MessageTextSurface);
	SDL_RenderPresent(renderer);
}
