#include <switch.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <AmiigoUI.h>
#include <chrono>
#include <thread>
#include "Utils.h"

extern int destroyer;
//Stolen from Goldleaf
//Thank you XOR
std::size_t CurlStrWrite(const char* in, std::size_t size, std::size_t num, std::string* out)
{
    const size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}

std::size_t CurlFileWrite(const char* in, std::size_t size, std::size_t num, FILE* out)
{
    fwrite(in, size, num, out);
    return (size * num);
}

std::string RetrieveContent(std::string URL, std::string MIMEType)
{
   std::string cnt;
    CURL *curl = curl_easy_init();
    if(!MIMEType.empty())
    {
        curl_slist *headerdata = NULL;
        headerdata = curl_slist_append(headerdata, ("Content-Type: " + MIMEType).c_str());
        headerdata = curl_slist_append(headerdata, ("Accept: " + MIMEType).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerdata);
    }
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo"); //Turns out this was important and I should not have deleted it
	curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlStrWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cnt);
    CURLcode res = curl_easy_perform(curl);	if (res != CURLE_OK) printf("\n%s\n",curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    return cnt;
}

void RetrieveToFile(std::string URL, std::string Path)
{
	CURLcode res = CURLE_OK;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	if (curl) {
		FILE *f = fopen(Path.c_str(), "wb");
		if(f)
		{
			curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
			curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlFileWrite);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
	fclose(f);
	}else{
        res = CURLE_HTTP_RETURNED_ERROR;			
	}
	if (res != CURLE_OK) printf("\n%s\n%s\n%s\n",curl_easy_strerror(res),URL.c_str(),Path.c_str());
}

//I made this so even though it's only one two calls it's probably janky.
std::string FormatURL(std::string TextToFormat)
{
	CURL *curl = curl_easy_init();
	return curl_easy_escape(curl, TextToFormat.c_str(), 0);
}

//More stuff from Xortroll Industries
bool HasConnection()
{
    u32 strg = 0;
	nifmInitialize(NifmServiceType_User);
    nifmGetInternetConnectionStatus(NULL, &strg, NULL);
	return (strg > 0);
}

void Scandownload(string folder)
{
	//Do the actual scanning
	DIR* dir;
	struct dirent* ent;
	dir = opendir(folder.c_str());
	while ((ent = readdir(dir)))
	{
		if (destroyer != 0) break;
		string route = ent->d_name;
		//Check if Amiibo or empty folder
		if(CheckFileExists(folder+"/"+route+"/amiibo.json"))
		{
			string imageI = folder+"/"+route+"/amiibo.png";
			string imageF = folder+"/"+route+"/amiibo_cache.png";
			if(!CheckFileExists(imageI))
			{
				//get id
				string APIContents;
				json APIJSData;
				ifstream IDReader(folder+"/"+route+"/amiibo.json");
				//Read each line
				for(int i = 0; !IDReader.eof(); i++)
				{
					string TempLine = "";
					getline(IDReader, TempLine);
					APIContents += TempLine;
				}
			
				IDReader.close();
				if(json::accept(APIContents))
				{
					APIJSData = json::parse(APIContents);
				}else
					return; //amiibo file is poison
					
				printf("Missing image %s \n",imageI.c_str());
				string AMID = toamii(APIJSData["id"]);
				string iconDBex = "sdmc:/config/amiigo/IMG/icon_"+AMID.substr(0,8)+"-"+AMID.substr(8,16)+".png";
				//if exist local used from there
				if(CheckFileExists(iconDBex)&(fsize(iconDBex) != 0)){
					printf("%s - Copy %s\nTo %s\n",route.c_str(),iconDBex.c_str(),imageI.c_str());
					copy_me(iconDBex, imageI);
					printf("Local used \n");
				} else if (HasConnection()){
					string url = "https://raw.githubusercontent.com/N3evin/AmiiboAPI/master/images/icon_"+AMID.substr(0,8)+"-"+AMID.substr(8,16)+".png";
					printf("%s - Downloading %s\nTo %s\n",route.c_str(),url.c_str(),imageI.c_str());
					RetrieveToFile(url, imageF);
					if (fsize(imageF) != 0)
					rename(imageF.c_str(), imageI.c_str());
					copy_me(imageI, iconDBex);
					printf("Downloaded \n");
				}
			}//else printf("The icon exist %s OK\n",route.c_str());
		}
		else 
		{
			if(!CheckFileExists(folder+"/"+route+"/amiibo.json"))
			Scandownload(folder+"/"+route);
		}
	}
}

void APIDownloader()
{
	printf("Open network Thread\n");
	mkdir("sdmc:/config/amiigo/", 1);
	mkdir("sdmc:/config/amiigo/IMG", 1);
	
	printf("Api Downloader\n");
	if(HasConnection())//Download Api
	{
	RetrieveToFile("https://www.amiiboapi.com/api/amiibo/", "sdmc:/config/amiigo/API.temp");
		if(CheckFileExists("sdmc:/config/amiigo/API.temp")&(fsize("sdmc:/config/amiigo/API.temp") != 0))
		{
			string AmiiboAPIString = "";
			ifstream DataFileReader("sdmc:/config/amiigo/API.temp");
			for(int f = 0; !DataFileReader.eof(); f++)
			{
				string TempLine = "";
				getline(DataFileReader, TempLine);
				AmiiboAPIString += TempLine;
			}
			DataFileReader.close();			
			if(json::accept(AmiiboAPIString))//check if download is a valid json
			{
				remove("sdmc:/config/amiigo/API.json");
				rename("sdmc:/config/amiigo/API.temp", "sdmc:/config/amiigo/API.json");
			} else {printf("API.temp invalid\n");}
		}
	}

	//Download amiibo icons
	printf("Icon Downloader\n");
	Scandownload("sdmc:/emuiibo/amiibo");
	
printf("Close Thread\n");
destroyer = 1;
}
