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


#ifndef H_Sequencer
#define H_Sequencer

#include <math.h>

#include "OSCListener.h"
#include "JoystickListener.h"
#include "MeshWarp.h"

#include <math.h>


#define NUM_TRACKS (6)
#define SAMPLERATE (44100)
#define FRAME_SIZE (64)
#define FRAME_DURATION ((double)FRAME_SIZE/SAMPLERATE)
#define MAX_PERIOD (999936/64)
#define MIN_PERIOD (1)

#define SEQ_SIZE 1024

// 10 minutes
#define IDLE_TIMEOUT (1000*60*10)

struct Frame {

    bool mSet;

    bool mMute;
    
    bool mBoost;
    bool mNoise;

    Real mYaw;
    Real mPitch;
    Real mRoll;
    
    Real mX;
    Real mY;

    Real mVolume;

    void set(bool isset, bool mute=false, bool boost=false, bool noise=false, 
            Real yaw=0.0, Real pitch=0.0, Real roll=0.0,
            Real x=0.0, Real y=0.0)
            //Real volume=0.0) 
    {
        mSet=isset;
        mMute=mute;
        mBoost=boost;
        mNoise=noise;
        mYaw=yaw;
        mPitch=pitch;
        mRoll=roll;
        mX=x;
        mY=y;
        //mVolume=volume;
    }
  
};

struct Track {
    TexturePtr mTex;
    HardwarePixelBufferSharedPtr mBuffer;
    MaterialPtr mMaterial;
    MaterialPtr mSelectedMaterial;
    SceneNode *mOffsetNode;
    SceneNode *mRotScaleNode;
    Real mVolume;
    Entity *mEntity;
    Timer *mTimer;
    bool mPlaying;
    Real mPhase;
    //Real mRPM;
    Real mPeriod; //in frames
    Real mLastUpdatePhase;

    String mOSCName;

    Frame mFrames[SEQ_SIZE];
};


       

class Sequencer: public FrameListener
{
    protected:

        // needed for compositors
        RenderWindow* mWindow;
		
        MaterialPtr mBaseMaterial;
        MaterialPtr mBaseSelectedMaterial;
        Entity *mBaseEntity;

        Entity *mGuideEntity;
       
        SceneNode *mGuideNode;
            
        SceneManager *mSceneMgr;
        Track mTracks[NUM_TRACKS];
        Real minVolScale, maxVolScale;

        SceneNode *mRotNode;
        SceneNode *mWarpNode;
        //SceneNode *extraNode;
        
        OSCListener *mOSCListener;
        JoystickListener *mJoystickListener;

        bool mRecording;
        bool mMute;
        bool mIdle;

        Timer *mIdleTimer;

        bool mNoise;
        bool mBoost;

        int mSelectedTrack;



        enum Mode 
        {
            MODE_UNKNOWN,
            MODE_TWIST, 
            MODE_TIMESTRETCH,
            MODE_RESAMPLE,
            MODE_VOLUME
        } mMode;

        // special entities for meshwarps
        Entity *mDiverHead;

        MeshWarpListener *mMeshWarpListener;

        SceneNode *mTwistAxes;

        Real mStretchBaseRPM; 
        Real mBaseVolume; 
        
        // filters

        LowPassFilter mNoiseLPF;
        LowPassFilter mBoostLPF;

        // compositors

        CompositorInstance *mBloom;
        CompositorInstance *mOldFarked;
        CompositorInstance *mSharpenEdges;
        CompositorInstance *mBlackAndWhite;
       
        // overlays
        
        Overlay *mRecordOverlay;
        Overlay *mMuteOverlay;
        Overlay *mTwistUsageOverlay;
        Overlay *mStretchUsageOverlay;
        Overlay *mTimeStretchCaptionOverlay;
        Overlay *mNoiseOverlay;
        Overlay *mVolumeUsageOverlay;
        Overlay *mBoostOverlay;
        Overlay *mTwistCaptionOverlay;
        Overlay *mLogoOverlay;

        // for preloading
        MeshPtr mHunk;
        MeshPtr mDiver;
        MeshPtr mMilk;
        MeshPtr mManowar;
        MeshPtr mPoints;
        MeshPtr mTurbulence;
        
        
    public:

        static void sendFrame(Frame &frame, OSCListener *mOSCListener, Track &track)
        {
            mOSCListener->send(track.mOSCName+"/twistAxis",frame.mYaw,frame.mPitch,frame.mRoll);
            mOSCListener->send(track.mOSCName+"/twistAngle",frame.mX,frame.mY);
            mOSCListener->send(track.mOSCName+"/mute",frame.mMute?1:0);
            mOSCListener->send(track.mOSCName+"/noise",frame.mNoise?1:0);
            mOSCListener->send(track.mOSCName+"/boost",frame.mBoost?1:0);
            //mOSCListener->send(track.mOSCName+"/volume",frame.mVolume);
        }

        static float rescale(Real in, Real in_low, Real in_high, Real out_low, Real out_high)
        {
            Real in_width = in_high-in_low;
            Real out_width = out_high-out_low;
            return out_low+((in-in_low)*out_width/(in_width));
        }
       
        static float symmetricLogScale(Real in, Real out_mul)
        {
            // the follwing function assumes x is [0,1] ..
            // but in is -1,1, so...

            Real x = (in+1.0)/2.0;
            Real out_min = 1.0/out_mul;
            Real out_max = out_mul;
            return out_min * exp(log(out_max/out_min)*x);
        }
        
        Sequencer(RenderWindow *win, SceneNode *sceneNode, String name, OSCListener *oscListener, JoystickListener *joystickListener)
            : mNoiseLPF(3), mBoostLPF(3)
        {
            mWindow = win;
            
            mOSCListener = oscListener;
            mJoystickListener = joystickListener;
            mSceneMgr = sceneNode->getCreator();
            mBaseEntity = mSceneMgr->createEntity(name+"ring","TorusUV3-5.mesh");
            mBaseMaterial = MaterialManager::getSingleton().getByName("fijuuu/dynHead");
            mBaseSelectedMaterial = MaterialManager::getSingleton().getByName("fijuuu/dyn3");

            mGuideEntity = mSceneMgr->createEntity(name+"guide","Guide.mesh");
           
            mRotNode = sceneNode->createChildSceneNode(); 
            mWarpNode = mRotNode->createChildSceneNode("warpNode");

            // preload
            mHunk = MeshManager::getSingleton().load("hunk.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mDiver = MeshManager::getSingleton().load("diver.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mMilk = MeshManager::getSingleton().load("milk.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mManowar = MeshManager::getSingleton().load("manowar.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mPoints = MeshManager::getSingleton().load("points.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mTurbulence = MeshManager::getSingleton().load("turbulence.mesh",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            
            mMeshWarpListener = new MeshWarpListener(mSceneMgr,mWarpNode);
            //mMeshWarpListener->setObject(mHunk->getName());

            // everyone else uses the mRoot object.. i hope this is the same thing.
            Root::getSingleton().addFrameListener(mMeshWarpListener);

            mMode = MODE_UNKNOWN;
            
            mRecording = mMute = 0;

            mIdle = 1;
            mIdleTimer = PlatformManager::getSingleton().createTimer();
            
            minVolScale = 1;
            maxVolScale = 2;
            
            //create the tracks
            for(int i=0;i<NUM_TRACKS;i++) {
                createTrack(
                        mTracks[i],
                        sceneNode,
                        name+"ring-"+StringConverter::toString(i),
                        i*3
                        ,"/track/"+StringConverter::toString(i));
                
            }

            resetAllTracks();
            
            createScene(sceneNode);

            // so that lastSelectedTrack is invalid
            mSelectedTrack = -1;
            selectTrack(0);
            
        }
        
        ~Sequencer()
        {
            delete mMeshWarpListener;
        }
       
        // here we mostly create things (entities, compositors) that get enabled/disabled later
        // like, on track changes
        void createScene(SceneNode *sceneNode)
        {
            mDiverHead = mSceneMgr->createEntity("diverHead","diverHead.mesh");

            mTwistAxes = mWarpNode->createChildSceneNode();
            Entity *axesEnt = mSceneMgr->createEntity("axes","fij_axes.mesh");
            // the mTwistAxes node is used by the meshwarp as the variable for storing the current twist,
            // so if you want to hide the axes, just comment out this next line to stop the mesh being attached.
            mTwistAxes->attachObject(axesEnt);
            mTwistAxes->setScale(0.1,0.1,0.1);
            axesEnt->setCastShadows(false);	


            SceneNode *sn;

            mGuideNode = mSceneMgr->createSceneNode();
          
            Node *n;
            if((n = mGuideNode->getParent())!=0)
            {
                LogManager::getSingleton().logMessage("guidenode has a parent? "+n->getName());
            }
            else
            {
                LogManager::getSingleton().logMessage("guidenode has no parent");
            }
                        

            
            sn = mGuideNode->createChildSceneNode();
            sn->setInheritScale(false);
            sn->setPosition(13,0.59,0);
            sn->attachObject(mGuideEntity->clone("guide0"));
            
            sn = mGuideNode->createChildSceneNode();
            sn->yaw(Radian(Math::HALF_PI));
            sn = sn->createChildSceneNode();
            sn->setInheritScale(false);
            sn->setPosition(13,0.59,0);
            sn->attachObject(mGuideEntity->clone("guide1"));

            sn = mGuideNode->createChildSceneNode();
            sn->yaw(Radian(Math::PI));
            sn = sn->createChildSceneNode();
            sn->setInheritScale(false);
            sn->setPosition(13,0.59,0);
            sn->attachObject(mGuideEntity->clone("guide2"));

            sn = mGuideNode->createChildSceneNode();
            sn->yaw(Radian(Math::PI+Math::HALF_PI));
            sn = sn->createChildSceneNode();
            sn->setInheritScale(false);
            sn->setPosition(13,0.59,0);
            sn->attachObject(mGuideEntity->clone("guide3"));


            // compositors
            
            mBloom = CompositorManager::getSingleton().addCompositor(mWindow->getViewport(0), "Bloom");
            mOldFarked = CompositorManager::getSingleton().addCompositor(mWindow->getViewport(0), "oldfarked");
            mSharpenEdges = CompositorManager::getSingleton().addCompositor(mWindow->getViewport(0), "Sharpen Edges");
            mBlackAndWhite= CompositorManager::getSingleton().addCompositor(mWindow->getViewport(0), "B&W");

            // overlays

            mRecordOverlay = OverlayManager::getSingleton().getByName("fijuuu/recordOverlay");
            mMuteOverlay = OverlayManager::getSingleton().getByName("fijuuu/muteOverlay");
            mTwistUsageOverlay = OverlayManager::getSingleton().getByName("fijuuu/useOverlay");
            mTwistCaptionOverlay = OverlayManager::getSingleton().getByName("fijuuu/twistCaptionOverlay");
            mStretchUsageOverlay = OverlayManager::getSingleton().getByName("fijuuu/stretchUsageOverlay");
            mTimeStretchCaptionOverlay = OverlayManager::getSingleton().getByName("fijuuu/timeStretchCaptionOverlay");
            mNoiseOverlay = OverlayManager::getSingleton().getByName("fijuuu/noiseOverlay");
            mVolumeUsageOverlay = OverlayManager::getSingleton().getByName("fijuuu/volumeUsageOverlay");
            mBoostOverlay = OverlayManager::getSingleton().getByName("fijuuu/boostOverlay");
            mLogoOverlay = OverlayManager::getSingleton().getByName("fijuuu/logoOverlay");

            mLogoOverlay->show();
        }

        void setMode(enum Mode mode) {
            if(mMode==mode)
                return;
            LogManager::getSingleton().logMessage("Switching from mode "+StringConverter::toString(mMode)+" to mode "+StringConverter::toString(mode));

            // cleanup
            switch(mMode)
            {
                case MODE_VOLUME:
                    // turn off volume overlays here
                    mVolumeUsageOverlay->hide();
                    break;
                
                case MODE_TWIST:
                    //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/twist",0);
                    // turn off twist usage overlay here
                    mTwistUsageOverlay->hide();
                    mTwistCaptionOverlay->hide();

                    break;
                case MODE_TIMESTRETCH:
                    //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/stretch",0);

                    // turn off timestrech usage overlay here
                    mStretchUsageOverlay->hide();
                    mTimeStretchCaptionOverlay->hide();
                    
                    break;
            }
            
            mMode=mode;

            // setup
            switch(mMode)
            {
                case MODE_VOLUME:
                    mBaseVolume = mTracks[mSelectedTrack].mVolume;
                    // turn on volume overlays here
					mVolumeUsageOverlay->show();
                    break;

                case MODE_TWIST:
                    //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/twist",1);
                    // turn on twist usage overlay here
                    mTwistUsageOverlay->show();
                    mTwistCaptionOverlay->show();

                    break;

                case MODE_TIMESTRETCH:
                    //mStretchBaseRPM = mTracks[mSelectedTrack].mRPM;
                    mStretchBaseRPM = getTrackRPM(mTracks[mSelectedTrack]);
                    //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/stretch",1);

                    // turn on overlay here
                    mStretchUsageOverlay->show();
					mTimeStretchCaptionOverlay->show();
                    
					break;
            }

        }
      
        /*
        void setTwistIdle(bool idle)
        {
            if(mTwistIdle==idle)
                retun;
        }
        */
        
        bool frameStarted(const FrameEvent &evt)
        {

            const Real &tslf = evt.timeSinceLastFrame;

            Track &track = mTracks[mSelectedTrack];

            JoystickListener *jl = mJoystickListener;

            Button &stretchButton = jl->getButton(1);
            Axis &timeAxis = jl->getAxis(0);
            Axis &pitchAxis = jl->getAxis(0);
            
            Button &clearButton = jl->getButton(3);
            Button &clearAllButton = jl->getButton(8);

            Button &volumeButton = jl->getButton(2);
            Axis &volumeAxis = jl->getAxis(1);

            Axis &twistAxis0 = jl->getAxis(0);
            Axis &twistAxis1 = jl->getAxis(1);
            Axis &twistAxis2 = jl->getAxis(2);
            Axis &twistAxis3 = jl->getAxis(3);

            Frame frames[NUM_TRACKS];
            Frame &frame = frames[mSelectedTrack];

            // read frames for all tracks
            for(int i=0;i<NUM_TRACKS;i++) {
                // ugly overloading
                Track &track = mTracks[i];
                Real phase = getTrackPhaseFromTimer(track);
                int index = iclip((int)(phase*SEQ_SIZE),0,SEQ_SIZE);
                Frame &frame = track.mFrames[index];
                if(frame.mSet)
                    frames[i]=track.mFrames[index];
                else
                    frames[i].set(false);
            }
            // send after this processing
            
            switch(mMode) 
            {
                case MODE_VOLUME:
                    if(!volumeButton.getState())
                    {
                        mIdleTimer->reset();
                        setMode(MODE_TWIST);
                    }

                    setTrackVolume(track,
                            -volumeAxis.getLPFPosition()+mBaseVolume);

                    break;
                case MODE_TIMESTRETCH:
                    if(!stretchButton.getState())
                    {
                       if(timeAxis.isDead()&&pitchAxis.isDead())
                       {
                        setMode(MODE_TWIST);
                        break;
                       }
                       else
                       {
                           // FIXME: setMode(MODE_RESAMPLE);
                           setMode(MODE_TWIST);
                       }
                    }

                    setTrackRPM(track,symmetricLogScale(-timeAxis.getPosition(),2.0)*mStretchBaseRPM);
                    
                    break;
                case MODE_TWIST:
               
                    if(stretchButton.getState())
                    {
                        mIdleTimer->reset();
                        setMode(MODE_TIMESTRETCH);
                        break;
                    }

                    if(volumeButton.getState())
                    {
                        mIdleTimer->reset();
                        setMode(MODE_VOLUME);
                        break;
                    }
                  
                    // this block is to allow temporary variable definitions without errors
                    {
                        bool isMuted = jl->getButton(4).getState();
                        bool isRecording = jl->getButton(5).getState();
                        bool boost = jl->getButton(6).getState();
                        bool noise = jl->getButton(7).getState();

                        mIdle = 
                            twistAxis0.isDead() &&
                            twistAxis1.isDead() &&
                            twistAxis2.isDead() &&
                            twistAxis3.isDead() &&
                            // err....
                            !isMuted && !boost && !noise;

                        if(!mIdle)
                            mIdleTimer->reset();
                        
                        if(clearButton.buttonDown())
                            clearTrack(track);

                        // reset if idle for 10 minutes.
                        if(clearAllButton.buttonDown() || (mIdle && mIdleTimer->getMilliseconds()>IDLE_TIMEOUT))
                        {
                            mIdleTimer->reset();
                            resetAllTracks();
                        }
                            //clearAllTracks();
                        
                        // we still use these external functions to update the frame
                        setRecording(isRecording);

                        // when there is a mute overlay, you don't want it coming on when you are in the deadspot.
                        // so we wil need two params here.. both send a mute message to pd but only one displays 
                        // the overlay

                        // we still use setMute to show overlays
                        setMute(isMuted);

                        
                        //setMute(idle || 
                        //        jl->getButton(4).getState());
                        
                        if(jl->getAxis(5).getNegativeButton().buttonDown()) selectNextTrack();
                        if(jl->getAxis(5).getPositiveButton().buttonDown()) selectPreviousTrack();


                        String trackNum = StringConverter::toString(mSelectedTrack);

                        mTwistAxes->yaw(Radian(jl->getAxis(0).getLPFPosition()*Math::TWO_PI*0.1*tslf));
                        mTwistAxes->pitch(Radian(jl->getAxis(3).getLPFPosition()*Math::TWO_PI*0.1*tslf));

                        //mOSCListener->send(track.mOSCName+"/twistAxis",o.getYaw().valueDegrees(),o.getPitch().valueDegrees(),o.getRoll().valueDegrees());

                        
                        setBoost(boost);
                        
                        Real boostRamp = mBoostLPF.write(boost?8:10,tslf);

                        setNoise(noise);
                        
                        mMeshWarpListener->setDisplacement(mNoiseLPF.write(noise?0.5:0,tslf));
                       
                        Axis &xTwistAxis = jl->getAxis(1);
                        Axis &yTwistAxis = jl->getAxis(2);
                        
                        //mOSCListener->send(track.mOSCName+"/twistAngle",xTwistAxis.getPosition(),yTwistAxis.getPosition());
                        
                        if(!mIdle || isRecording) {
                            frame.mMute = isMuted;
                            frame.mBoost = boost;
                            frame.mNoise = noise;

                            Quaternion o = mTwistAxes->getOrientation(); 
                            frame.mYaw = o.getYaw().valueDegrees();
                            frame.mPitch = o.getPitch().valueDegrees();
                            frame.mRoll = o.getRoll().valueDegrees();

                            frame.mX = xTwistAxis.getPosition();
                            frame.mY = yTwistAxis.getPosition();
                            
                        }

                        // there is now the possibility to set these things from the recording..
                        // well not quite, since we loose the axes, but we could... if...
                       
                        mMeshWarpListener->setTwist(0,
                                mTwistAxes->getOrientation().xAxis(),
                                xTwistAxis.getLPFPosition()*Math::PI/(boostRamp),
                                0);
                        
                        mMeshWarpListener->setTwist(1,
                                mTwistAxes->getOrientation().yAxis(),
                                yTwistAxis.getLPFPosition()*Math::PI/(boostRamp),
                                0);

                        //frame.send(mOSCListener,track);

                        /*
                        if(isRecording) {
                            updateTrackSeq(track,frame);
                        }
                        */
                    }
                    break;
                default:
                    LogManager::getSingleton().logMessage("unknown mode...");
                    setMode(MODE_TWIST);
                    break;
            }

            // rotate the meshWarp
            mRotNode->yaw(Degree(1)*evt.timeSinceLastFrame);

            // update the seq.. should we do this all the time?
            update(frame);

            for(int i=0;i<NUM_TRACKS;i++) {
                Track &track = mTracks[i];
                sendFrame(frames[i],mOSCListener,track);
                
                mOSCListener->send(track.mOSCName+"/volume",track.mVolume);
            }
            
            return true;
        }
        
        void setRecording(bool recording)
        {
            if(recording==mRecording)
                return;
            mRecording = recording;

            if(mRecording) 
            {
                mRecordOverlay->show();
				mBlackAndWhite->setEnabled(true);

            }
            else
            {
                mRecordOverlay->hide();
		if(!mMute) mBlackAndWhite->setEnabled(false);
            }

            Track &track = mTracks[mSelectedTrack];
            //mOSCListener->send(track.mOSCName+"/recording",mRecording?1.0:0.0,getTrackPhaseFromTimer(track));


        }
        
        void setMute(bool mute)
        {
            /* i'll just put these notes here..
             * with the old method, if you were idle, and there was nothing recorded, you would hear nothing
             *
             * i guess it's the same now if we full the seq full of mutes at startup
             */
            
            if(mute==mMute)
                return;
            mMute = mute;

            if(!mMute) {
                // clean up 
                // remember to turn off here anything you turn on below..
                mMuteOverlay->hide();
                mBloom->setEnabled(false);
		if(!mRecording) mBlackAndWhite->setEnabled(false);
            }
            else
            {
                mBloom->setEnabled(true);
		mBlackAndWhite->setEnabled(true);
                mMuteOverlay->show();
            }


            //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/mute",mMute?1.0:0.0);
        }
       
        void setNoise(bool noise)
        {
            if(noise==mNoise)
                return;
            mNoise=noise;

            //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/noise",mNoise?1:0);

            if(!mNoise) {
                // clean up 
                // remember to turn off here anything you turn on below..
                mOldFarked->setEnabled(false);
                //			mSharpenEdges->setEnabled(false);
                mNoiseOverlay->hide();
            }
            else
            {
                mOldFarked->setEnabled(true);
                //			mSharpenEdges->setEnabled(true);
                mNoiseOverlay->show();
            }
        }
        
        void setBoost(bool boost)
        {
            if(boost==mBoost)
                return;
            mBoost=boost;

            //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/boost",mBoost?1:0);

            if(!mBoost) {
                // clean up 
                // remember to turn off here anything you turn on below..
                mBoostOverlay->hide();
                mSharpenEdges->setEnabled(false);
            }
            else
            {
                mSharpenEdges->setEnabled(true);
                mBoostOverlay->show();
            }
        }

        void clearTrack(Track &track) {
            track.mBuffer->lock(HardwareBuffer::HBL_NORMAL);
            const PixelBox &pb = track.mBuffer->getCurrentLock();
            uint32 *data = static_cast<uint32*>(pb.data);
            
            for(unsigned int x=0; x<SEQ_SIZE; x++)
            {
                /*
                // temporarily fill with random data to make it visible
                if(Math::UnitRandom()>0.5)
                PixelUtil::packColour(1.0f,1.0f,1.0f,1.0f,PF_A8R8G8B8,&data[x]);
                else
                */
                PixelUtil::packColour(0.0f,0.0f,0.0f,0.0f,PF_A8R8G8B8,&data[x]);
                track.mFrames[x].set(false);
            }
            
                
            track.mBuffer->unlock();
            
        }
        

        void clearAllTracks() 
        {
            for(int i=0;i<NUM_TRACKS;i++)
                clearTrack(mTracks[i]);
        }

        void resetAllTracks()
        {
            for(int i=0;i<NUM_TRACKS;i++)
            {
                Track &track = mTracks[i]; 
                clearTrack(track);
                setTrackVolume(track,0.5);
                setTrackRPM(track,Math::RangeRandom(5,10));
            }
        }
        
        int imax(int a, int b)
        {
            return a>b?a:b;
        }
        
        int imin(int a, int b)
        {
            return a<b?a:b;
        }
        
        int iclip(int n, int low, int high)
        {
            return imin(imax(n, low), high);
        }

        Real clip(Real f, Real low, Real high)
        {
            return (f<low?low:(f>high?high:f));
        }
        
        void updateTrackTextureAndSeq(Track &track,Frame frame) {
            
            Real currentPhase = getTrackPhaseFromTimer(track);
            
            if(mRecording)
            {
                //printf("rec or erase\n");
                
                if(track.mLastUpdatePhase<0.0)
                {
                    
                    //printf("unknown last update phase\n");
                    track.mLastUpdatePhase=currentPhase;
                }
                
                int start = iclip((int)(track.mLastUpdatePhase*SEQ_SIZE),0,SEQ_SIZE);
                int end = iclip((int)(currentPhase*SEQ_SIZE),0,SEQ_SIZE);
                
                if(start>end)
                {
                    end+=SEQ_SIZE;
                }
                
                //printf("start=%d end=%d\n");
                
                track.mBuffer->lock(HardwareBuffer::HBL_NORMAL);
                const PixelBox &pb = track.mBuffer->getCurrentLock();
                uint32 *data = static_cast<uint32*>(pb.data);
                
                for(unsigned int x=start; x<end; x++)
                {
                    int mx = x%SEQ_SIZE;
                    if(mRecording)
                    {
                        frame.mSet=true;
                        track.mFrames[mx]=frame;
                        if(mMute || mIdle)
                            PixelUtil::packColour(0.0f,0.0f,0.0f,0.0f,PF_A8R8G8B8,&data[mx]);
                        else
                            PixelUtil::packColour(1.0f,1.0f,1.0f,1.0f,PF_A8R8G8B8,&data[mx]);
                    }
                }
                
                track.mBuffer->unlock();
                
                track.mLastUpdatePhase=currentPhase;
            } 
            else
            {
                track.mLastUpdatePhase=-1.0;
            } 
            
            
            
        }
       
        void createTrackSeq(Track &track) {
            for(int i=0;i<SEQ_SIZE;i++) {
                Frame &frame = track.mFrames[i];

                frame.mSet=false;
                frame.mMute=true;
                frame.mBoost=false;
                frame.mNoise=false;
                frame.mYaw=0.0;
                frame.mPitch=0.0;
                frame.mRoll=0.0;
                frame.mX=0.0;
                frame.mY=0.0;
            }
        }
       
        void setRecordMaskTexture(MaterialPtr m, String name, TextureType tt)
        {
            for(Material::TechniqueIterator ti = m->getTechniqueIterator() ;
                    ti.hasMoreElements() ; ti.moveNext())
                for(Technique::PassIterator pi = ti.peekNext()->getPassIterator() ;
                        pi.hasMoreElements() ; pi.moveNext())
                    for (Pass::TextureUnitStateIterator tusi = pi.peekNext()->getTextureUnitStateIterator() ;
                            tusi.hasMoreElements() ; tusi.moveNext())
                    {
                        TextureUnitState *tus = tusi.peekNext();
                        if(StringUtil::startsWith(tus->getName(),"record_mask"))
                        {
                            tus->setTextureName(name,tt);
                        }
                    }
        }
        
        void createTrackMaterial(Track &track, String name) {
            // Create dynamic texture
            track.mTex = TextureManager::getSingleton().createManual(
                    name,"General", TEX_TYPE_1D, SEQ_SIZE, 1, 0, PF_A8R8G8B8,
                    //TU_AUTOMIPMAP);
                TU_DYNAMIC_WRITE_ONLY);
            track.mBuffer = track.mTex->getBuffer(0, 0);
            
            track.mMaterial = mBaseMaterial->clone(name);


            setRecordMaskTexture(track.mMaterial,name,TEX_TYPE_1D);
            
            track.mSelectedMaterial = mBaseSelectedMaterial->clone(name+"-selected");
            setRecordMaskTexture(track.mSelectedMaterial,name,TEX_TYPE_1D);
            
            clearTrack(track);
        }
        
        /*
           Real wrap(Real f) {
           return f-((int)f);
           }
           */
        
        Real getTrackPhaseFromTimer(Track &track) {
            Real junk;
            if(track.mPlaying)
            {
                //return modff(track.mPhase + (track.mTimer->getMicroseconds()*track.mRPM/60000000.0),&junk);
                return modff(track.mPhase + (track.mTimer->getMicroseconds()/(track.mPeriod*FRAME_DURATION*1000000.0)),&junk);
            }
            else
            {
                return track.mPhase;
            }
        }
        
        // use timer to update track phase
        void updateTrackPhase(Track &track) {
            Real phase = getTrackPhaseFromTimer(track);
            // FIXME: unnecessary now
            // mOSCListener->send(track.mOSCName+"/period",track.mPeriod,phase);
            applyTrackPhase(track,phase);
        }
       
        void setTrackPeriod(Track &track, int period) 
        {
            //printf("set track period %d\n",period);
            Real currentPhase = getTrackPhaseFromTimer(track);
            track.mTimer->reset();
            track.mPeriod = iclip(period,MIN_PERIOD,MAX_PERIOD);
            track.mPhase = currentPhase;

            // mOSCListener->send(track.mOSCName+"/period",track.mPeriod,track.mPhase);
        }
       
        Real getTrackRPM(Track &track)
        {
            Real rpm = (60.0/(track.mPeriod*FRAME_DURATION));
            //printf("gettrackrpm==%f\n",rpm);
            return rpm;
        }
        
        void setTrackRPM(Track &track, Real RPM)
        {
            //printf("set track rpm %f, frame duration %f\n",RPM,FRAME_DURATION);
            setTrackPeriod(track,(int)(round(60.0/(RPM*FRAME_DURATION))));
        }

        void startTrack(Track &track)
        {
            if(!track.mPlaying)
            {
                track.mTimer->reset();
                track.mPlaying=true;
            }
        }

        void stopTrack(Track &track)
        {
            Real currentPhase = getTrackPhaseFromTimer(track);
            track.mTimer->reset();
            track.mPhase = currentPhase;
            track.mPlaying=false;
        }

        
        Real linp (Real n1, Real n2, Real a) 
        {
            return n1+a*(n2-n1);
        }
    
        void applyTrackVolume(Track &track)
        {
            Real scaleFactor = linp(minVolScale,maxVolScale,track.mVolume);
            track.mRotScaleNode->setScale(scaleFactor,1,scaleFactor);
        }
      
        void rotateAllTextureUnits(MaterialPtr material, Real phase) 
        {
            for(Material::TechniqueIterator ti = material->getTechniqueIterator() ;
                    ti.hasMoreElements() ; ti.moveNext())
                for(Technique::PassIterator pi = ti.peekNext()->getPassIterator() ;
                        pi.hasMoreElements() ; pi.moveNext())
                    for (Pass::TextureUnitStateIterator tusi = pi.peekNext()->getTextureUnitStateIterator() ;
                            tusi.hasMoreElements() ; tusi.moveNext())
                    {
                        TextureUnitState *tus = tusi.peekNext();
                        //printf("texturename:%s\n",tus->getTextureName().c_str());
                        if(!StringUtil::startsWith(tus->getName(),"static"))
                        {
                            //printf("rotating\n");
                            tus->setTextureUScroll(phase);
                        }
                    }
        }


        void applyTrackPhase(Track &track, Real phase)
        {
            //Radian rot = Radian(Math::TWO_PI/phase);
            rotateAllTextureUnits(track.mMaterial,phase);
            rotateAllTextureUnits(track.mSelectedMaterial,phase);
            //track.mGuideRotScaleNode->resetOrientation();
            //track.mGuideRotScaleNode->yaw(Radian(-phase*Math::TWO_PI));
        }

        
        void setTrackVolume(Track &track, Real volume)
        {
            track.mVolume = clip(volume,0.0,1.0);
            applyTrackVolume(track);

            mOSCListener->send(track.mOSCName+"/volume",track.mVolume);
        }

        void selectTrack(int n)
        {
            if(mSelectedTrack==n) 
                return;

            setRecording(false);
            setMute(false);

            int lastSelectedTrack = mSelectedTrack;
            mSelectedTrack=n;
           
            //mOSCListener->send(mTracks[lastSelectedTrack].mOSCName+"/select",0);
            //mOSCListener->send(mTracks[mSelectedTrack].mOSCName+"/select",1);

            // stop using n here..
            
            

            for(int i=0;i<NUM_TRACKS;i++)
            {
                Track &track = mTracks[i];
                if(i==mSelectedTrack)
                {
                    track.mEntity->setMaterialName(track.mSelectedMaterial->getName());
                    track.mLastUpdatePhase=-1;
                }
                else
                {
                    track.mEntity->setMaterialName(track.mMaterial->getName());
                }
            }

            // clean up 
            //
            // remember to turn off here anything you turn on below..
            switch(lastSelectedTrack+1) {
                case 1:
                    //mBloom->setEnabled(false);
					break;
                case 2:
                    //mBloom->setEnabled(false);
                    mWarpNode->detachObject(mDiverHead);
                    break;
                case 3:
                    //mBloom->setEnabled(false);
  //                  mSharpenEdges->setEnabled(false);
                    break;
                case 4:
                    //mBloom->setEnabled(false);
                    break;
                case 5:
                    //mBloom->setEnabled(false);
                    mMeshWarpListener->getObjectEntity()->setCastShadows(true);
                    break;
                case 6:
                    //mBloom->setEnabled(false);
                    break;
            }
           
            if(lastSelectedTrack>=0)
                mTracks[lastSelectedTrack].mRotScaleNode->removeChild(mGuideNode);
            mTracks[mSelectedTrack].mRotScaleNode->addChild(mGuideNode);
          
            mWarpNode->setPosition(0,10,0);
            //LogManager::getSingleton().logMessage("setting up mesh "+StringConverter::toString(mSelectedTrack));
           
            // set up
            switch(mSelectedTrack+1) {
                case 1:
                    //mBloom->setEnabled(true);
                    //mMeshWarpListener->setObject("hunk.mesh");
                    mMeshWarpListener->setObject(mHunk->getName());
                    break;
                case 2:
                    //mBloom->setEnabled(true);
                    //mMeshWarpListener->setObject("diver.mesh");
                    mMeshWarpListener->setObject(mDiver->getName());
                    mWarpNode->attachObject(mDiverHead);
                    mWarpNode->setPosition(-7,10,0);
                    break;
                case 3:
                    //mBloom->setEnabled(true);
                    //mMeshWarpListener->setObject("milk.mesh");
                    mMeshWarpListener->setObject(mMilk->getName());
//                    mSharpenEdges->setEnabled(true);
                    break;
                case 4:
                    //mBloom->setEnabled(true);
                    //mMeshWarpListener->setObject("manowar.mesh");
                    mMeshWarpListener->setObject(mManowar->getName());
                    //mWarpNode->setPosition(0,10,-5);
                    mWarpNode->setPosition(0,10,0);
					//mWarpNode->pitch(Degree(90));
                    break;
                case 5:
                    //mBloom->setEnabled(true);
                    //mMeshWarpListener->setObject("points.mesh");
                    mMeshWarpListener->setObject(mPoints->getName());
                    mMeshWarpListener->getObjectEntity()->setCastShadows(false);
					break;
                case 6:
                    //mBloom->setEnabled(true);
                    mWarpNode->setPosition(0,15,0);
                    mMeshWarpListener->setObject(mTurbulence->getName());
                    //mMeshWarpListener->setObject("turbulence.mesh");
                    break;
            }
        }

        void selectNextTrack()
        {
            selectTrack((mSelectedTrack+1)%NUM_TRACKS);
        }
        
        void selectPreviousTrack()
        {
            selectTrack((mSelectedTrack+NUM_TRACKS-1)%NUM_TRACKS);
        }
        
        void createTrack(Track &track, SceneNode *sceneNode, String name, Real offset, String oscName)
        {
            track.mOSCName = oscName;
            track.mPlaying=false;
            //track.mRPM=0;
            track.mTimer = PlatformManager::getSingleton().createTimer();
            track.mPhase=0.0;

            track.mLastUpdatePhase = -1.0;
           

            createTrackSeq(track);
            
            createTrackMaterial(track,name);

            track.mOffsetNode = sceneNode->createChildSceneNode();
            track.mOffsetNode->setPosition(0,offset,0);
            track.mRotScaleNode = track.mOffsetNode->createChildSceneNode();



            track.mEntity = mBaseEntity->clone(name);
            track.mEntity->setMaterialName(track.mMaterial->getName());

            track.mRotScaleNode->attachObject(track.mEntity);

            

            setTrackVolume(track,0.5);
            
            setTrackRPM(track,Math::RangeRandom(5,10));
            startTrack(track);
        }
        
        void destroyTrackMaterial(Track &track)
        {
            track.mTex.setNull();
            track.mBuffer.setNull();
        }

        void destroyTrack(Track &track)
        {
            destroyTrackMaterial(track);
        }

        void destroyScene(void)
        {
            //destroy the tracks
            for(int i=0;i<NUM_TRACKS;i++) {
                //printf("destroying track %d\n",i);
                destroyTrack(mTracks[i]);
            }

        }

        void update(Frame &frame) 
        {
            for(int i=0;i<NUM_TRACKS;i++) {
                updateTrackPhase(mTracks[i]);
            }
            Track &track = mTracks[mSelectedTrack];
            updateTrackTextureAndSeq(track,frame);
            mGuideNode->resetOrientation();
            mGuideNode->yaw(Radian(-getTrackPhaseFromTimer(track)*Math::TWO_PI));
        }

    
};



#endif
