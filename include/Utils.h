#include <string>
#include <sys/stat.h>
#include "nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;

int fsize(string fil);
bool CheckFileExists(string);
string GoUpDir(string);
bool copy_me(string origen, string destino);
void DrawJsonColorConfig(SDL_Renderer* renderer, string Head);
json toemu(std::string ID);
std::string toamii(json JSID);
