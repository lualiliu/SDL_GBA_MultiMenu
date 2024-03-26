/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/
//The headers
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <libintl.h>
using namespace std;
//Screen attributes
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 160;
const int SCREEN_BPP = 32;
//The surfaces
SDL_Surface *background = NULL;
SDL_Surface *message = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *noroms = NULL;
//The event structure
SDL_Event event;
//The font that's going to be used
TTF_Font *font = NULL;
//The color of the font
SDL_Color font_color = {
	0, 0, 0
}
;
//
vector<string> GamesList;
//显示的游戏标题
SDL_Surface* Gamestitle[8];
//当前选择项和当前页
int Select = 0;
int Page = 0;
int Total = 0;
//页的总数
int PageTotal = 0;
//页的模，用来获取最后一页剩下多少项。假如为8项，则该值为8,并且此时总页数-1,选项放满8个
int SelectEndMod = 0;
//游戏列表标题文本坐标
SDL_Rect GamestitleRectdst[8] = { {
		28,24+14*0,0,0
	}
	, {
		28,24+14*1,0,0
	}
	, {
		28,24+14*2,0,0
	}
	, {
		28,24+14*3,0,0
	}
	, {
		28,24+14*4,0,0
	}
	, {
		28,24+14*5,0,0
	}
	, {
		28,24+14*6,0,0
	}
	, {
		28,24+14*7,0,0
	}
}
;
string TotalText = "";
SDL_Surface* TotalTextSurface = NULL;
SDL_Rect TotalTextRectdst = {
	195,143,0,0
}
;
void getGameList() {
	//获取游戏列表到vector
	GamesList.clear();
	//清除元素
	struct dirent *ptr;
	string tmpstr = "./roms";
	DIR *dir=opendir(tmpstr.c_str());
	if(dir == NULL) return;
	while((ptr=readdir(dir))!=NULL) {
		//跳过'.'和'..'两个目录GamesList[i]
		if(ptr->d_name[0] == '.')
					continue;
		if(ptr->d_type==DT_REG) {
			string tmpNode;
			tmpstr = ptr->d_name;tmpNode = tmpstr.substr(0,tmpstr.rfind("."));
			if((tmpstr.find(".gba")) != string::npos) {
				GamesList.push_back(tmpNode);
			}
		}
	}
	sort(GamesList.begin(),GamesList.end());
	// for (int i = 0; i < GamesList.size(); ++i)
	//{
	// cout << GamesList[i] << endl;
	//}
	//cout<<GamesList.size()<<endl;
	PageTotal = GamesList.size() / 8;
	SelectEndMod = GamesList.size() % 8;
	if(SelectEndMod == 0) {
		PageTotal-=1;
		SelectEndMod = 8;
	}
	//else
	//PageTotal++;
	for (int i = 0;i<GamesList.size();i++) {
		if(i<8) {
			if (Gamestitle[i]) {
				SDL_FreeSurface(Gamestitle[i]);
			}
			Gamestitle[i] = TTF_RenderUTF8_Blended(font ,GamesList[Page*8+i].c_str(),font_color);
		} else {
			break;
		}
	}
	return;
}
SDL_Surface *load_image( std::string filename ) {
	//The image that's loaded
	SDL_Surface* loadedImage = NULL;
	//The optimized surface that will be used
	SDL_Surface* optimizedImage = NULL;
	//Load the image
	loadedImage = IMG_Load( filename.c_str() );
	//If the image loaded
	if( loadedImage != NULL ) {
		//Create an optimized surface
		optimizedImage = SDL_DisplayFormat( loadedImage );
		//Free the old surface
		SDL_FreeSurface( loadedImage );
		//If the surface was optimized
		if( optimizedImage != NULL ) {
			//Color key surface
			SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
		}
	}
	//Return the optimized surface
	return optimizedImage;
}
void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL ) {
	//Holds offsets
	SDL_Rect offset;
	//Get offsets
	offset.x = x;
	offset.y = y;
	//Blit
	SDL_BlitSurface( source, clip, destination, &offset );
}
bool init() {
	//Initialize all SDL subsystems
	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
		return false;
	}
	//Set up the screen
	screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
	//If there was an error in setting up the screen
	if( screen == NULL ) {
		return false;
	}
	//Initialize SDL_ttf
	if( TTF_Init() == -1 ) {
		return false;
	}
	//Set the window caption
	SDL_WM_SetCaption( "GBA_MultiMenu", NULL );
	//If everything initialized fine
	return true;
}
bool load_files() {
	//Load the background image
	background = load_image( "bg.png" );
	//Open the font
	font = TTF_OpenFont( "font.ttf", 12 );
	//If there was a problem in loading the background
	if( background == NULL ) {
		return false;
	}
	//If there was an error in loading the font
	if( font == NULL ) {
		return false;
	}
	//If everything loaded fine
	return true;
}
void clean_up() {
	//Free the surfaces
	SDL_FreeSurface( background );
	SDL_FreeSurface( message );
	//Close the font that was used
	TTF_CloseFont( font );
	//Quit SDL_ttf
	TTF_Quit();
	//Quit SDL
	SDL_Quit();
}
int main( int argc, char* args[] ) {
	//Quit flag
	bool quit = false;
	//Initialize
	if( init() == false ) {
		return 1;
	}
	//Load the files
	if( load_files() == false ) {
		return 1;
	}
	//Render the text
	message = TTF_RenderUTF8_Blended( font, "⇨", font_color );
	//Render the text
	noroms = TTF_RenderUTF8_Blended( font, "No ROMs", font_color );
	//If there was an error in rendering the text
	if( message == NULL ) {
		return 1;
	}
	getGameList();
	//for (int i = 0; i < GamesList.size(); ++i) {
	//	cout << GamesList[i] << endl;
	//}
	//cout<<GamesList.size()<<endl;
	//While the user hasn't quit
	while( quit == false ) {
		if(GamesList.size()==0) {
			//While there's events to handle
			while( SDL_PollEvent( &event ) ) {
				//If the user has Xed out the window
				if( event.type == SDL_QUIT ) {
					//Quit the program
					quit = true;
				}
			}
			apply_surface( 0, 0, background, screen );
			apply_surface( 90, 70, noroms, screen );
		} else {
			//While there's events to handle
			while( SDL_PollEvent( &event ) ) {
				//If the user has Xed out the window
				if( event.type == SDL_QUIT ) {
					//Quit the program
					quit = true;
				}
				if( event.type == SDL_KEYDOWN ) {
					if(event.key.keysym.sym==SDLK_UP) {
						if(Page == 0 && Select <= 0){
						
						}
						else if ( Select > 0 )
							Select -= 1;
						else {
							Select = 7;
							if ( Page > 0 ) {
								Page -= 1;
							}
						}
					} else if(event.key.keysym.sym==SDLK_DOWN) {
						if(Page == PageTotal && Select > SelectEndMod-2){
						
						}				
						else if ( Select < 8 - 1 )
							Select += 1;
						else {
							Select = 0;
							if ( Page < PageTotal ) {
								Page += 1;
							}
						}
					} else if(event.key.keysym.sym==SDLK_RETURN) {
						pid_t pid = fork();
        					string cmd;
						cmd = "./roms/"+GamesList[Page * 8 + Select]+".gba";
						if(pid == 0){
							cout<<cmd<<endl;
							//execlp("gpsp","gpsp",cmd.c_str(),NULL);
						}else{
							pause();
						}
					}

					for ( int i = 0; i < GamesList.size(); i++ ) {
							if ( Page == PageTotal ) {
								if ( i < SelectEndMod ) {
									if (Gamestitle[i]) {
										SDL_FreeSurface(Gamestitle[i]);
									}
									Gamestitle[i] = TTF_RenderUTF8_Blended( font, GamesList[Page * 8 + i].c_str(), font_color );
									for ( int j = i + 1; j < 8; j++ ) {
										if (Gamestitle[j]) {
											SDL_FreeSurface(Gamestitle[j]);
										}
										Gamestitle[j] = TTF_RenderUTF8_Blended( font, "", font_color );
									}
								}
							} else if ( i < 8 ) {
								if (Gamestitle[i]) {
									SDL_FreeSurface(Gamestitle[i]);
								}
								Gamestitle[i] = TTF_RenderUTF8_Blended( font, GamesList[Page * 8 + i].c_str(), font_color );
							} else {
								break;
							}
					}
				}
				//draw background
				apply_surface( 0, 0, background, screen );
				for ( int i = 0; i < 8; i++ )
				{
					if ( Gamestitle[i] != NULL )
					{
						apply_surface( GamestitleRectdst[i].x,GamestitleRectdst[i].y,Gamestitle[i], screen );
					}
				}

				if (TotalTextSurface) {
					SDL_FreeSurface(TotalTextSurface);
				}
				TotalText = to_string(Select+1+Page*8)+"/"+to_string(GamesList.size());
				TotalTextSurface = TTF_RenderUTF8_Blended( font, TotalText.c_str(), font_color );
				apply_surface( TotalTextRectdst.x,TotalTextRectdst.y,TotalTextSurface, screen );

				apply_surface( 14, 24+Select*14, message, screen );
				
			}

		}
			//Update the screen
			if( SDL_Flip( screen ) == -1 ) {
				return 1;
			}
		}
		//Free surfaces and font then quit SDL_ttf and SDL
	clean_up();
	return 0;
}
