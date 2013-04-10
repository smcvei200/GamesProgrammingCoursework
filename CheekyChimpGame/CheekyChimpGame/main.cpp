/*
=================
main.cpp
Main entry point for the Card application
=================
*/

#include "GameConstants.h"
#include "GameResources.h"
#include "cD3DManager.h"
#include "cD3DXSpriteMgr.h"
#include "cD3DXTexture.h"
#include "cBanana.h"
#include "cSprite.h"
#include "cExplosion.h"
#include "cXAudio.h"
#include "cD3DXFont.h"

using namespace std;

HINSTANCE hInst; // global handle to hold the application instance
HWND wndHandle; // global variable to hold the window handle

// Get a reference to the DirectX Manager
static cD3DManager* d3dMgr = cD3DManager::getInstance();

// Get a reference to the DirectX Sprite renderer Manager 
static cD3DXSpriteMgr* d3dxSRMgr = cD3DXSpriteMgr::getInstance();

//set initial starting position for the monkey
D3DXVECTOR2 monkeyTrans = D3DXVECTOR2(300,450);

//Global variables
vector<cBanana*> aBanana;
vector<cBanana*>::iterator iter;
vector<cBanana*>::iterator index;

cSprite theMonkey;

RECT clientBounds;
RECT monkeyBounds;

TCHAR szTempOutput[30];

int mouseClicks = 0;
bool gHit = false;
int gScore = 0;
char gScoreStr[50];
D3DXVECTOR3 expPos;
list<cExplosion*> gExplode;
float speed = 50.0f;
cXAudio gExplodeSound;
cXAudio gMonkeySound;
cXAudio gThemeSound;
int bananaSquash;
bool gameOver = false;

cD3DXTexture* txtExp = new cD3DXTexture();



cD3DXTexture* BananaTextures[2];
char* BananaTxtres[] = {"Images\\banana.png","Images\\BananaSquash.png"};

/*
==================================================================
* LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
* LPARAM lParam)
* The window procedure
==================================================================
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messages from the queue
	switch (message)
	{
		case WM_KEYDOWN:
			{
				if (wParam == VK_LEFT)
				{
					//if left key is pressed then check if monkey is past left bounds
					if(monkeyTrans.x > (clientBounds.left))
					{
						//move monkey left at a speed of 5
					monkeyTrans.x -= 5.0f;
					return 0;
					}
				}
				if (wParam == VK_RIGHT)
				{
					//if right key is pressed then check if monkey is past right bounds
					if(monkeyTrans.x < (clientBounds.right-30))
					{
						//move monkey right at a speed of 5
					monkeyTrans.x += 5.0f;
					return 0;
					}
				}
				return 0;
			}
		
		case WM_LBUTTONDOWN:
			{
				//increment mouseClicks
				mouseClicks++;
				
				return 0;
			}
		case WM_CLOSE:
			{
			// Exit the Game
				PostQuitMessage(0);
				 return 0;
			}

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
	}
	// Always return the message to the default window
	// procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
==================================================================
* bool initWindow( HINSTANCE hInstance )
* initWindow registers the window class for the application, creates the window
==================================================================
*/
bool initWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;
	// Fill in the WNDCLASSEX structure. This describes how the window
	// will look to the system
	wcex.cbSize = sizeof(WNDCLASSEX); // the size of the structure
	wcex.style = CS_HREDRAW | CS_VREDRAW; // the class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; // the window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this instance
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MyWindowIcon)); // icon to associate with the application
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_GUNSIGHT));// the default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // the background color
	wcex.lpszMenuName = NULL; // the resource name for the menu
	wcex.lpszClassName = "CheekyChimp"; // the class name being created
	wcex.hIconSm = LoadIcon(hInstance,"theMonkey.ico"); // the handle to the small icon

	RegisterClassEx(&wcex);
	// Create the window
	wndHandle = CreateWindow("CheekyChimp",			// the window class to use
							 "CheekyChimp",			// the title bar text
							WS_OVERLAPPEDWINDOW,	// the window style
							CW_USEDEFAULT, // the starting x coordinate
							CW_USEDEFAULT, // the starting y coordinate
							800, // the pixel width of the window
							600, // the pixel height of the window
							NULL, // the parent window; NULL for desktop
							NULL, // the menu for the application; NULL for none
							hInstance, // the handle to the application instance
							NULL); // no values passed to the window
	// Make sure that the window handle that is created is valid
	if (!wndHandle)
		return false;
	// Display the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	UpdateWindow(wndHandle);
	return true;
}

/*
==================================================================
// This is winmain, the main entry point for Windows applications
==================================================================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	// Initialize the window
	if ( !initWindow( hInstance ) )
		return false;
	// called after creating the window
	if ( !d3dMgr->initD3DManager(wndHandle) )
		return false;
	if ( !d3dxSRMgr->initD3DXSpriteMgr(d3dMgr->getTheD3DDevice()))
		return false;

	

	// Grab the frequency of the high def timer
	__int64 freq = 0;				// measured in counts per second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	float sPC = 1.0f / (float)freq;			// number of seconds per count

	__int64 currentTime = 0;				// current time measured in counts per second;
	__int64 previousTime = 0;				// previous time measured in counts per second;

	float numFrames   = 0.0f;				// Used to hold the number of frames
	float timeElapsed = 0.0f;				// cumulative elapsed time

	GetClientRect(wndHandle,&clientBounds);

	float fpsRate = 1.0f/25.0f;

	//variables to hold the monkey, banana and new banana positions
	D3DXVECTOR3 monkeyPos;
	D3DXVECTOR3 BananaPos;
	D3DXVECTOR3 NewPos;

	//string to hold the score and display on screen
	sprintf_s( gScoreStr, 50, "Score : %d", gScore);

	//===========================================================
	// load texture for the bananas
	//===========================================================
	for (int txture = 0; txture < 1; txture++)
	{
		BananaTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), BananaTxtres[txture]);
	}
	
	//create a texture for the explosion
	txtExp->createTexture(d3dMgr->getTheD3DDevice(),"Images\\explosion.png");

	//create a texture for the monkey
	cD3DXTexture* txtMonkey = new cD3DXTexture();
	txtMonkey->createTexture(d3dMgr->getTheD3DDevice(),"Images\\theMonkey.png");
	txtMonkey->setTextureInfo("Images\\theMonkey.png");//set the texture information ie. height, width
	
	theMonkey.setTranslation(D3DXVECTOR2(0.0f, 0.0f));//set the translation of the monkey
	theMonkey.setTexture(txtMonkey);//apply the texture to the monkey
	
	theMonkey.update();//update the monkey sprite
	
	/* initialize random seed: */
	srand ( (unsigned int)time(NULL) );
	
	//integer to hold the number of bananas
	int numBananas =  1;

	for(int loop = 0; loop < numBananas; loop++)//for each banana set the position, direction, speed and texture
	{
		BananaPos = D3DXVECTOR3((float)clientBounds.right/(2),(float)clientBounds.top-100,0);
		aBanana.push_back(new cBanana());
		aBanana[loop]->setSpritePos(BananaPos);
		aBanana[loop]->setTranslation(D3DXVECTOR2(0.0f,speed));
		aBanana[loop]->setTexture(BananaTextures[loop]);
		
	}
	
	


	LPDIRECT3DSURFACE9 TitleSurface;//the title surface
	LPDIRECT3DSURFACE9 GameSurface;		//the main game background
	LPDIRECT3DSURFACE9 GameOverSurface;	// the game over surface
	LPDIRECT3DSURFACE9 theBackbuffer = NULL;  // This will hold the back buffer
	
	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	// Create the background surface
	TitleSurface = d3dMgr->getD3DSurfaceFromFile("Images\\TitleScreen.png");
	GameSurface = d3dMgr->getD3DSurfaceFromFile("Images\\GameBackground.png");
	GameOverSurface = d3dMgr->getD3DSurfaceFromFile("Images\\GameOverSurface.png");
	// load custom font
	cD3DXFont* balloonFont = new cD3DXFont(d3dMgr->getTheD3DDevice(),hInstance, "SNAP_");

	//rectangle to hold the score during gameplay
	RECT textPos;
	SetRect(&textPos, 50, 10, 400, 100);

	//rectangle to hold the score on the gameover screen
	RECT textPos2;
	SetRect(&textPos2, 250, 250, 600, 400);

	QueryPerformanceCounter((LARGE_INTEGER*)&previousTime);

	while( msg.message!=WM_QUIT )
	{
		// Check the message queue
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			// Game code goes here
			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
			float dt = (currentTime - previousTime)*sPC;

			// Accumulate how much time has passed.
			timeElapsed += dt;
			
			if(timeElapsed > fpsRate)
			{

				//start rendering 
				d3dMgr->beginRender();
					
				if(mouseClicks==0)//if mouseClicks is equal to zero
				{
					
				theBackbuffer = d3dMgr->getTheBackBuffer();
				d3dMgr->updateTheSurface(TitleSurface, theBackbuffer);//apply the title surface
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				gameOver=false;//set gameOver to false
				gScore = 0;//set gScore to zero
				sprintf_s( gScoreStr, 50, "Score : %d", gScore);//set the score string with zero
				}
				else 			
				{
					if(bananaSquash == 3)//if bananaSquash is equal to three
					{
						
						gameOver = true;//set gameOver to 
						mouseClicks = -1;// set mouseClicks to negative one
						bananaSquash = 0;//set bananaSquash to zero
						speed = 50.0f;//set the speed to 50
						
						
					}
					else
					{
						if(gameOver  == true)//if gameOver is true
						{
							theBackbuffer = d3dMgr->getTheBackBuffer();
							d3dMgr->updateTheSurface(GameOverSurface, theBackbuffer);//apply the game over surface
							d3dMgr->releaseTheBackbuffer(theBackbuffer);
							sprintf_s( gScoreStr, 50, "You Scored : %d", gScore);//update the score 
							balloonFont->printText(gScoreStr,textPos2);//print the score on the gameOver surface
						}
						else
						{
						
							vector<cBanana*>::iterator iterBanana = aBanana.begin();
			
						while(iterBanana != aBanana.end())
						{
					

							(*iterBanana)->update(timeElapsed);			// update banana
							BananaPos = (*iterBanana)->getSpritePos();//get the position of the banana
				
				
						if((*iterBanana)->collidedWith((*iterBanana)->getBoundingRect(), theMonkey.getBoundingRect()))	//check if monkey and banana have collided	
						{
							
							OutputDebugString("Collision!!");//if they have collided output collision 
							
							iterBanana = aBanana.erase(iterBanana);//delete banana
							gScore++;//increment the score
							sprintf_s( gScoreStr, 50, "Score : %d", gScore);//update the score string
							speed = speed+3;//add three to the speed
							gMonkeySound.playSound(L"Sounds\\monkeySound2.wav",false);//play monkey sound effect
						
				}
				else
				{
					if (BananaPos.y >(clientBounds.bottom-60 ))//if the banana hits the floor
					{
						expPos = (*iterBanana)->getSpritePos();//get the banana position
						gExplode.push_back(new cExplosion(expPos,txtExp));	//play explosion animation at same position as banana					
						gExplodeSound.playSound(L"Sounds\\explosion.wav",false);//play explosion sound
						iterBanana = aBanana.erase(iterBanana);//delete the banana
						OutputDebugString("Collision!!");//output collision 
						bananaSquash++;//increment bananaSquash
						
					}
					
					else
					{
						
						++iterBanana;// increment the iterator
						OutputDebugString("ITERBanana");//display ITERbanana
											
					}
					
				}									
				}

				int vSize=aBanana.size();//integer to hold the size of aBanana
				BananaPos = aBanana[vSize-1]->getSpritePos();//variable to hold the position of the banana
				if(BananaPos.y>(clientBounds.bottom-300))//check if the banana has fallen past about 3 quarters of the way down the screen
				{
					int xPos =  (rand() % 600 + 1);//integer variable to hold a random number for the new bananas x-position
					NewPos = D3DXVECTOR3((float)xPos,(float)clientBounds.top-100,0);//variable to hold new bananas starting position
					aBanana.push_back(new cBanana());//create a new banana
					vSize=aBanana.size();//get the size of aBanana
					aBanana[vSize-1]->setSpritePos(NewPos);//set the position for the new banana
					aBanana[vSize-1]->setTranslation(D3DXVECTOR2(0.0f,speed));//set the speed and direction of the new banana
					aBanana[vSize-1]->setTexture(BananaTextures[0]);//apply a texture to the new banana

				}
				
				
				monkeyPos = D3DXVECTOR3(monkeyTrans.x,monkeyTrans.y,0);//variable to hold the position of the monkey
				theMonkey.setSpritePos(monkeyPos);//set the position of the monkey
				
				theMonkey.update();//update the monkey sprite


					theBackbuffer = d3dMgr->getTheBackBuffer();//retrieve the backbuffer
				d3dMgr->updateTheSurface(GameSurface, theBackbuffer);//apply the game surface
				d3dMgr->releaseTheBackbuffer(theBackbuffer);//release the backbuffer
				d3dxSRMgr->beginDraw();//start drawing the window
				
				d3dxSRMgr->setTheTransform(theMonkey.getSpriteTransformMatrix());//set the position of the monkey sprite
				d3dxSRMgr->drawSprite(theMonkey.getTexture(),NULL,NULL,NULL,0xFFFFFFFF);// draw the monkey
			
				vector<cBanana*>::iterator iterB = aBanana.begin();
				for(iterB = aBanana.begin(); iterB != aBanana.end(); ++iterB)//for each banana in aBanana
				{
					d3dxSRMgr->setTheTransform((*iterB)->getSpriteTransformMatrix());  //set the position of the banana
					d3dxSRMgr->drawSprite((*iterB)->getTexture(),NULL,NULL,NULL,0xFFFFFFFF);//draw the banana
				
				}
				list<cExplosion*>::iterator iter = gExplode.begin();//set the iterator to begin
				while(iter != gExplode.end())//while the iterator is not set to end
				{
					if((*iter)->isActive() == false)//if the iterator is not active 
					{
						iter = gExplode.erase(iter);//delete the explosion
					}
					else
					{
						(*iter)->update(timeElapsed);
						d3dxSRMgr->setTheTransform((*iter)->getSpriteTransformMatrix());  //set the position for the explosion
						d3dxSRMgr->drawSprite((*iter)->getTexture(),&((*iter)->getSourceRect()),NULL,NULL,0xFFFFFFFF);//draw the explosion
						++iter;//increment iter
					}
				}
				balloonFont->printText(gScoreStr,textPos);//print the score 
				}
				}
				d3dxSRMgr->endDraw();//finish drawing
				
				}
				d3dMgr->endRender();//finish rendering
				OutputDebugString("timeElapsed > fpsRate");
				timeElapsed = 0.0f;//set timeElapsed to zero
			}

			previousTime = currentTime;
			/*
			StringCchPrintf(szTempOutput, 30, TEXT("dt=%f\n"), dt);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("timeElapsed=%f\n"), timeElapsed);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("previousTime=%u\n"), previousTime);
			OutputDebugString(szTempOutput);
			StringCchPrintf(szTempOutput, 30, TEXT("fpsRate=%f\n"), fpsRate);
			OutputDebugString(szTempOutput);
			*/
		}
	}
	d3dxSRMgr->cleanUp();
	d3dMgr->clean();
	return (int) msg.wParam;
}
