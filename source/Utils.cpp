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
string IDContents;
json JEData;

using namespace std;

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

void DrawJsonColorConfig(SDL_Renderer* renderer, string Head)
{
	if(CheckFileExists("sdmc:/config/amiigo/config.json"))
	{
		if (IDContents.size() == 0)
		{
			ifstream IDReader("sdmc:/config/amiigo/config.json");
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
				remove("sdmc:/config/amiigo/bad_config.json");
				rename("sdmc:/config/amiigo/config.json","sdmc:/config/amiigo/bad_config.json");
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

//tryng to tranlate the emuiibo id from amiibo id, WIP
json toemu(std::string ID){
	json JSID;
/*	
    printf("hex val: %s\n", ID.substr(0,4).c_str());
    std::string converted = shiftAndDec(ID.substr(0,4));
    printf("Number: %s\n", converted.c_str());
	
    std::string uconverted = shiftAndHex(converted);
//	transform(uconverted.begin(), uconverted.end(), uconverted.begin(), ::tolower);
    printf("get back: %s\n", uconverted.c_str());
*/
	printf("AmiiboID: %s \n",ID.c_str());

	JSID["game_character_id"] = std::stoi( shiftAndDec( ID.substr(0,4) ) );
	JSID["character_variant"] = std::stoi( ConvertBase(ID.substr(4,2),  16, 10) );
	JSID["figure_type"] = std::stoi( ConvertBase(ID.substr(6,2),  16, 10) );
	JSID["model_number"] = std::stoi( ConvertBase(ID.substr(8,4),  16, 10) );
	JSID["series"] = std::stoi( ConvertBase(ID.substr(12,2),  16, 10) );


	//this is for debug
	printf("%d ---- %s \n",JSID["game_character_id"].get<int>(),ID.substr(0,4).c_str());
	printf("%d ---- %s \n",JSID["character_variant"].get<int>(),ID.substr(4,2).c_str());
	printf("%d ---- %s \n",JSID["figure_type"].get<int>(),ID.substr(6,2).c_str());
	printf("%d ---- %s \n",JSID["model_number"].get<int>(),ID.substr(8,4).c_str());
	printf("%d ---- %s \n",JSID["series"].get<int>(),ID.substr(12,2).c_str());
	return JSID;
}

std::string toamii(json JSID)
{
	string game_character_id = refill(shiftAndHex( std::to_string(JSID["game_character_id"].get<int>()) ),4);
	string character_variant = refill(ConvertBase( std::to_string(JSID["character_variant"].get<int>()),10,16),2);
	string figure_type = refill(ConvertBase( std::to_string(JSID["figure_type"].get<int>()),10,16),2);
	string model_number = refill(ConvertBase( std::to_string(JSID["model_number"].get<int>()),10,16),4);
	string series = refill(ConvertBase( std::to_string(JSID["series"].get<int>()),10,16),2);
	
	
	//this is for debug
	printf("%s ---- %s \n",std::to_string( JSID["series"].get<int>() ).c_str(),series.c_str());
	printf("%s ---- %s \n",std::to_string( JSID["model_number"].get<int>() ).c_str(),model_number.c_str());
	printf("%s ---- %s \n",std::to_string( JSID["figure_type"].get<int>() ).c_str(),figure_type.c_str());
	printf("%s ---- %s \n",std::to_string( JSID["character_variant"].get<int>() ).c_str(),character_variant.c_str());
	printf("%s ---- %s \n",std::to_string( JSID["game_character_id"].get<int>() ).c_str(),game_character_id.c_str());
	
	
	string AMIIDD = 
	game_character_id+
	character_variant+
	figure_type+
	model_number+
	series+
	"02";
	printf("AmiiboID: %s -\n",AMIIDD.c_str());
	return AMIIDD;
}