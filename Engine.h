/*
    CutsceneEditor source - Engine.h
    Copyright (c) 2014 Mark Hutcheson
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "globaldefs.h"
#include "Image.h"
#include "Object.h"
#include "Text.h"
#include "hud.h"
#include "tyrsound.h"
#include <map>
#include <set>

#define LMB	1
#define RMB	0
#define MMB 2

typedef struct
{
	string sSwitch, sValue;
} commandlineArg;

class depthComparator
{
public:
    bool operator()(const physSegment* s1, const physSegment* s2)
    {
        return s1->depth < s2->depth;
    }
};

class Engine
{
private:
    //Variables for use by the engine
	string m_sTitle;
	string m_sIcon;
	SDL_Window* m_Window;
	list<commandlineArg> lCommandLine;
    b2World* m_physicsWorld;
    Point m_ptCursorPos;
    bool  m_bShowCursor;
    float32 m_fFramerate;
    float32 m_fAccumulatedTime;
    float32 m_fTargetTime;
	list<obj*> m_lObjects;	//Object handler
	multiset<physSegment*, depthComparator> m_lScenery;
	//TODO: Sound handler
    //map<string, string> m_mSoundNames; //And names of sounds
    //bool m_bFirstMusic; //Don't stop a previous song playing if there is none
    //string m_sLastMusic;    //Last song we played, so we can pause/resume songs instead of restarting them
    bool m_bQuitting;   //Stop the game if this turns true
	float32 m_fTimeScale;	//So we can scale time if we want
    uint16_t m_iWidth, m_iHeight;
    const Uint8 *m_iKeystates;    //Keep track of keys that are pressed/released so we can poll as needed
    int m_iNumScreenModes;      //Number of screen modes that are available
    bool m_bFullscreen;
	bool m_bResizable;
	float32 m_fGamma;		//Overall screen brightness
	bool m_bPaused;			//If the game is paused due to not being focused
	bool m_bPauseOnKeyboardFocus;	//If the game pauses when keyboard focus is lost
	
	map<string, tyrsound_Handle> m_sounds;

    //Engine-use function definitions
    bool _frame();
    void _render();
    
    void setup_sdl();
    void setup_opengl();
	void _loadicon();					//Load icon and set window to have said icon

    Engine(){}; //Default constructor isn't callable

protected:

    //Functions to override in your own class definition
    virtual void frame(float32 dt) = 0;   //Function that's called every frame
    virtual void draw() = 0;    //Actual function that draws stuff
    virtual void init(list<commandlineArg> sArgs) = 0;    //So we can load all our images and such
    virtual void handleEvent(SDL_Event event) = 0;  //Function that's called for each SDL input event
	virtual obj* objFromXML(string sXMLFilename, Point ptOffset, Point ptVel) = 0;	//Function called when an object should be created
	
	//Helper functions for your own class definition
	b2World* getWorld() {return m_physicsWorld;};

public:
    //Constructor/destructor
    Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable = false);
    ~Engine();

    //Methods
	void commandline(int argc, char** argv);	//Pass along commandline arguments for the engine to use
    void start();   //Runs engine and doesn't exit until the engine ends
    /*void fillRect(Point p1, Point p2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha); //Fill the specified rect with the specified color
    void fillRect(float32 x1, float32 y1, float32 x2, float32 y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
    void fillRect(Rect rc, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
	void fillRect(float32 x1, float32 y1, float32 x2, float32 y2, Color col);*/
    void createSound(string sPath, string sName);   //Creates a sound from this name and file path
    virtual void playSound(string sName, int volume = 100, int pan = 0, float32 pitch = 1.0);     //Play a sound
    void playMusic(string sName, int volume = 100, int pan = 0, float32 pitch = 1.0);     //Play looping music, or resume paused music
    void pauseMusic();                                                                     //Pause music that's currently playing
	void resumeMusic();																		//Resume music that was paused
	void restartMusic();
	void seekMusic(float32 fTime);
    bool keyDown(int32_t keyCode);  //Test and see if a key is currently pressed
    void quit() {m_bQuitting = true;};  //Stop the engine and quit nicely
    Rect getScreenRect()    {Rect rc = {0,0,getWidth(),getHeight()}; return rc;};
    b2Body* createBody(b2BodyDef* bdef) {return m_physicsWorld->CreateBody(bdef);};
    Point getCursorPos()    {return m_ptCursorPos;};
    void setCursorPos(int32_t x, int32_t y);
    void setCursorPos(Point ptPos)  {setCursorPos(ptPos.x, ptPos.y);};
	bool getCursorDown(int iButtonCode);
	void showCursor()	{SDL_ShowCursor(1);};
	void hideCursor()	{SDL_ShowCursor(0);};
	bool isMouseGrabbed()	{return SDL_GetWindowGrab(m_Window);};
	void grabMouse(bool bGrab = true) {SDL_SetWindowGrab(m_Window, (SDL_bool)bGrab);};
    void setGravity(Point ptGravity)    {m_physicsWorld->SetGravity(ptGravity);};
    void setGravity(float32 x, float32 y)   {setGravity(Point(x,y));};
    void changeScreenResolution(float32 w, float32 h);  //Change resolution mid-game and reload OpenGL textures as needed
    void toggleFullscreen();                            //Switch between fullscreen/windowed modes
	void setFullscreen(bool bFullscreen);				//Set fullscreen to true or false as needed
	bool isFullscreen()	{return m_bFullscreen;};
	bool isMaximized();	
	Point getWindowPos();	//Get the window position
	void setWindowPos(Point pos);	//Set window position
	void maximizeWindow();								//Call window manager to maximize application window
    list<SDL_DisplayMode> getAvailableResolutions();         //Get available fullscreen resolutions
	float32 getTimeScale()	{return m_fTimeScale;};
	void setTimeScale(float32 fScale)	{m_fTimeScale = fScale;};
	void addObject(obj* o);
	void addScenery(physSegment* seg) 	{m_lScenery.insert(seg);};
	void drawObjects();
	void cleanupObjects();
	void updateObjects(float32 dt);
	void stepPhysics(float32 dt)    {m_physicsWorld->Step(dt * m_fTimeScale, VELOCITY_ITERATIONS, PHYSICS_ITERATIONS);};
	void pauseOnKeyboard(bool p)	{m_bPauseOnKeyboardFocus = p;};
	bool pausesOnFocusLost()		{return m_bPauseOnKeyboardFocus;};

    //Accessor methods
    void setFramerate(float32 fFramerate);
    float32 getFramerate()   {return m_fFramerate;};
    uint16_t getWidth() {return m_iWidth;};
    uint16_t getHeight() {return m_iHeight;};
	void setDoubleBuffered(bool bDoubleBuffered)	{SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, bDoubleBuffered);};
	bool getDoubleBuffered()	{int val = 1; SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &val); return val;};
	void setVsync(int iVsync)	{SDL_GL_SetSwapInterval(iVsync);};
	int getVsync()				{return SDL_GL_GetSwapInterval();};
	int getMSAA()				{int MSAA = 0; SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &MSAA); return MSAA;};
	void setMSAA(int iMSAA);	
	void setGamma(float32 fGamma)	{m_fGamma = fGamma;};
	float32 getGamma()				{return m_fGamma;};

};


#endif