/*
 * This file is part of fijuu2 (a.k.a fijuuu).
 * Copyright (C) 2006 Steven Pickles, Julian Oliver
 * 
 * fijuu2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * fijuu2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ExampleApplication.h"

#include "Sequencer.h"
//#include "MeshWarp.h"
#include "OSCListener.h"
#include "JoystickListener.h"

//#include "SDL/SDL.h"

class SimpleListener : public FrameListener
{
    protected:
        InputReader *mInputReader;
    public:
        SimpleListener(RenderWindow *win)
        {
            mInputReader = PlatformManager::getSingleton().createInputReader();
            // grab mouse in fullscreen mode (otherwise we don't get key focus)
            mInputReader->initialise(win,true,win->isFullScreen());
        }

        bool frameStarted(const FrameEvent &evt)
        {
            mInputReader->capture();

            if(mInputReader->isKeyDown(KC_ESCAPE))
                return false;

            return true;
        }
};

class FijuuApplication : public ExampleApplication
{
protected:
    OSCListener *mOSCListener;

    Sequencer *mSequencer;
    
public:
    FijuuApplication()
    {
    }

    ~FijuuApplication() 
    {
    }

#if not(DEBUG_INPUT)

    virtual void createFrameListener(void)
    {
        mRoot->addFrameListener(new SimpleListener(mWindow));
    }

#endif

    virtual void go(void)
    {
#ifdef FRAMELIMIT  
        struct timespec ts;
       
        // this needs to be <1
        unsigned long min_frame_microseconds = 1000000/FRAMELIMIT;
#endif      
        
        if (!setup())
            return;

        LogManager::getSingleton().logMessage("render system: "+mRoot->getRenderSystem()->getName());

        // set to 1 to run at full framerate
#ifdef FRAMELIMIT    
        mRoot->getRenderSystem()->_initRenderTargets();
        
        Timer *timer = PlatformManager::getSingleton().createTimer();
        
        ts.tv_sec = 0;
        
        while(true) {
            timer->reset();
            if(!mRoot->renderOneFrame())
                break;
            unsigned long frame_microseconds = timer->getMicroseconds();
            if (frame_microseconds<min_frame_microseconds) {
                ts.tv_sec = 0;
                ts.tv_nsec = (min_frame_microseconds-frame_microseconds)*1000;
                nanosleep(&ts,NULL);
            }
        }
        
        PlatformManager::getSingleton().destroyTimer(timer);
#else
        mRoot->startRendering();
#endif
        
        // clean up
        destroyScene();
    }

protected:
   


    virtual void createCamera(void)
    {
        mCamera = mSceneMgr->createCamera("Camera");
        //mCamera->setPosition(Vector3(10,35,30));
        //mCamera->setPosition(Vector3(12.3,62.17,10.34));
        
		//mCamera->setPosition(Vector3(19.24,60.52,19.53));
        //mCamera->lookAt(Vector3(0,20,0));
        
		//good asymmetrical composition
		//mCamera->setPosition(Vector3(38.81,65.8,12.67));
        //mCamera->lookAt(Vector3(0,10,0));
  		
		//also good. centered meshwarp, fuller scene. 
		mCamera->setPosition(Vector3(31.19,62.26,11.49));
        mCamera->lookAt(Vector3(0,10,0));
		
		mCamera->setNearClipDistance(5);
		//mCamera->setFOVy(Degree(45));
    }

    virtual void createViewports(void)
    {
    	Viewport* vp = mWindow->addViewport(mCamera);
		vp->setBackgroundColour(ColourValue(0,0,0));
		mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    }



    void createScene(void)
    {
       
        mOSCListener = new OSCListener(4177,"osc.udp://localhost:4178/");
      
        mRoot->setFrameSmoothingPeriod(2.0);
        
        JoystickListener *mJoystickListener = new JoystickListener();
        mRoot->addFrameListener(mJoystickListener);

        SceneNode *seqNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("seq");
        seqNode->setPosition(0,10,0);
		seqNode->yaw(Degree(120));

        mSequencer = new Sequencer(mWindow, seqNode, "seq", mOSCListener, mJoystickListener);



	   Entity *room;
	   room = mSceneMgr->createEntity("room", "room.mesh");
       SceneNode *roomNode= mSceneMgr->getRootSceneNode()->createChildSceneNode("room");
	   mSceneMgr->getSceneNode("room")->attachObject(room);
       room->setCastShadows(false);
       roomNode->setPosition(0,20,0);

	   Entity *readhead;
	   readhead = mSceneMgr->createEntity("readhead", "readhead.mesh");
	   SceneNode *readheadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("readhead");
	   mSceneMgr->getSceneNode("readhead")->attachObject(readhead);
	   readhead->setCastShadows(false);
	   readheadNode->setPosition(0,16,0);
	   readheadNode->yaw(Degree(120));
	   
	   Entity *brace;
	   brace = mSceneMgr->createEntity("brace", "brace.mesh");
	   mSceneMgr->getSceneNode("readhead")->attachObject(brace);
	 
	   Entity *braceArm;
	   braceArm = mSceneMgr->createEntity("braceArm", "braceArm.mesh");
	   mSceneMgr->getSceneNode("readhead")->attachObject(braceArm);
                       

       Light *light;
       //mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
       mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
       mSceneMgr->setShadowTextureSize(1024);
       mSceneMgr->setAmbientLight(ColourValue(0.0,0.0,0.0));
       //mSceneMgr->setAmbientLight(ColourValue::Black);
       light = mSceneMgr->createLight("Light1");
       //light->setType(Light::LT_POINT);
       light->setType(Light::LT_SPOTLIGHT);
       light->setSpotlightRange(Degree(25),Degree(90));
       light->setCastShadows(true);
       light->setDirection(Vector3::NEGATIVE_UNIT_Y);
       light->setPosition(Vector3(10,80,10));
       light->setDiffuseColour(0.6,0.6,0.9);
       light->setSpecularColour(0.0,0.0,.9);
       //light->setAttenuation(8000,1,0.0005,0);
      


       

       mRoot->addFrameListener(mOSCListener);
       
       mRoot->addFrameListener(mSequencer);
       
    }

    void destroyScene(void) {
        mSequencer->destroyScene();
    }
};

#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    FijuuApplication app;

    try {

        SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE );
        SDL_JoystickEventState( SDL_ENABLE );
        
        app.go();

        printf("sdlquit..\n");
        SDL_Quit();
        
    } catch( Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }

    return 0;
}

