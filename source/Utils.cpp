#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <switch.h>
#include <vector>
#include <stdio.h>
#include <chrono>
#include "Utils.h"
#include "ConvertBase.hpp"

using namespace std;
string IDContents;
json JEData;
extern std::string cfgroot;

int fsize(string fil) {
 std::streampos fsize = 0;

    std::ifstream myfile (fil, ios::in);  // File is of type const char*

    fsize = myfile.tellg();         // The file pointer is currently at the beginning
    myfile.seekg(0, ios::end);      // Place the file pointer at the end of file

    fsize = myfile.tellg() - fsize;
    myfile.close();

    static_assert(sizeof(fsize) >= sizeof(long long), "Oops.");
    return fsize;
}

bool CheckFileExists(string Path)
{
	struct stat Buffer;   
	return (stat (Path.c_str(), &Buffer) == 0); 
}

string GoUpDir(string Path)
{
	Path = Path.substr(0,Path.find_last_of("/\\"));
	Path = Path.substr(0,Path.find_last_of("/\\")+1);
	printf("%s\n",Path.c_str());
	if (Path.size() < 21) return "sdmc:/emuiibo/amiibo/";
	return Path;
}

/*
* copy function
*/
bool copy_me(string origen, string destino) {
    if(CheckFileExists(origen))
	{
		ifstream source(origen, ios::binary);
		ofstream dest(destino, ios::binary);
		dest << source.rdbuf();
		source.close();
		dest.close();
		return true;
	}else{
		return false;
	}
return 0;
}
//tryng to tranlate the emuiibo id from amiibo id, WIP
json toemu(std::string ID){
	json JSID;
	printf("AmiiboID: %s \n",ID.c_str());
	JSID["game_character_id"] = std::stoi( shiftAndDec( ID.substr(0,4) ) );
	JSID["character_variant"] = std::stoi( ConvertBase(ID.substr(4,2),  16, 10) );
	JSID["figure_type"] = std::stoi( ConvertBase(ID.substr(6,2),  16, 10) );
	JSID["model_number"] = std::stoi( ConvertBase(ID.substr(8,4),  16, 10) );
	JSID["series"] = std::stoi( ConvertBase(ID.substr(12,2),  16, 10) );
	return JSID;
}

std::string toamii(json JSID)
{
	//get and convert the values in json and concatenate to form the ID
	string ID = 
	refill(shiftAndHex( std::to_string(JSID["game_character_id"].get<int>()) ),4)+
	refill(ConvertBase( std::to_string(JSID["character_variant"].get<int>()),10,16),2)+
	refill(ConvertBase( std::to_string(JSID["figure_type"].get<int>()),10,16),2)+
	refill(ConvertBase( std::to_string(JSID["model_number"].get<int>()),10,16),4)+
	refill(ConvertBase( std::to_string(JSID["series"].get<int>()),10,16),2)+
	"02";
	printf("AmiiboID: %s -\n",ID.c_str());
	return ID;
}
/*
void DrawJsonColorConfig(SDL_Renderer* renderer, string Head)
{
	if(CheckFileExists(cfgroot+"config.json"))
	{
		if (IDContents.size() == 0)
		{
			ifstream IDReader(cfgroot+"config.json");
				//Read each line
				printf("Read Json\n");
				for(int i = 0; !IDReader.eof(); i++)
				{
					string TempLine = "";
					getline(IDReader, TempLine);
					IDContents += TempLine;
					printf("%s\n", TempLine.c_str());
				}
			IDReader.close();
			if(json::accept(IDContents))
			{
				printf("Parse\n");
				JEData = json::parse(IDContents);
				printf("Parse OK\n");
			}else{
				//remove bad config
				IDContents = "";
				remove((cfgroot+"bad_config.json").c_str());
				rename((cfgroot+"config.json").c_str(),(cfgroot+"bad_config.json").c_str());
			}
		}else{
//		printf("%s \n",Head.c_str());
		int CR = std::stoi(JEData[Head+"_R"].get<std::string>());
		int	CG = std::stoi(JEData[Head+"_G"].get<std::string>());
		int	CB = std::stoi(JEData[Head+"_B"].get<std::string>());
		int	CA = std::stoi(JEData[Head+"_A"].get<std::string>());
//		if (CA != 0)
//		printf("%s %d %d %d %d\n",Head.c_str(),CR,CG,CB,CA);
		SDL_SetRenderDrawColor(renderer,CR,CG,CB,CA);
		}
	}else{
		//Default values
		if(Head == "UI_borders") SDL_SetRenderDrawColor(renderer,0 ,0 ,0 ,255);
		if(Head == "UI_borders_list") SDL_SetRenderDrawColor(renderer,0 ,0 ,0 ,255);
		if(Head == "UI_background") SDL_SetRenderDrawColor(renderer,136 ,254 ,254 ,255);
		if(Head == "UI_background_alt") SDL_SetRenderDrawColor(renderer,0 ,178 ,212 ,255);
		if(Head == "UI_cursor") SDL_SetRenderDrawColor(renderer,255 ,255 ,255 ,255);
		if(Head == "AmiigoUI_DrawUI") SDL_SetRenderDrawColor(renderer,94 ,94 ,94 ,255);
		if(Head == "AmiigoUI_DrawHeader") SDL_SetRenderDrawColor(renderer,0 ,188 ,212 ,255);
		if(Head == "AmiigoUI_PleaseWait") SDL_SetRenderDrawColor(renderer,0 ,188 ,212 ,255);
		if(Head == "AmiigoUI_DrawFooter_0") SDL_SetRenderDrawColor(renderer,0 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_1") SDL_SetRenderDrawColor(renderer,255 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_2") SDL_SetRenderDrawColor(renderer,255 ,0 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_3") SDL_SetRenderDrawColor(renderer,255 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_D") SDL_SetRenderDrawColor(renderer,255 ,0 ,0 ,255);
		if(Head == "CreatorUI_DrawHeader") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if(Head == "CreatorUI_DrawFooter_Select") SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		if(Head == "CreatorUI_DrawFooter_Back") SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		if(Head == "CreatorUI_PleaseWait") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if(Head == "CreatorUI_DrawUI") SDL_SetRenderDrawColor(renderer, 94, 94, 94, 255);
		if(Head == "UpdaterUI_DrawText") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if (IDContents.size() != 0) IDContents = "";//reset json var
	}
}
*/

