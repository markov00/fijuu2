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

#ifndef H_JoystickListener
#define H_JoystickListener

#include <SDL/SDL.h>


class Button
{
    protected:
        bool mState, mOldState;

    public:
        Button()
        {
            mState = false;
            mOldState = false;
        }

        void setState(bool state) 
        {
            mOldState = mState;
            mState = state;
        }

        bool getState()
        {
            return mState;
        }

        bool getOldState()
        {
            return mOldState;
        }

        bool buttonDown()
        {
            return (mState&&!mOldState);
        }
        
        bool buttonUp()
        {
            return (!mState&&mOldState);
        }
};

class LowPassFilter
{
    protected:
        Real mCutoff;
        Real mLast;
        Real mCurrent;
    public:
        LowPassFilter(Real cutoff)
        {
            mCutoff = cutoff;
            mLast = 0.0f;
            mCurrent= 0.0f;
        }

        void setCutoff(Real cutoff)
        {
            mCutoff = cutoff;
        }

        Real write(Real v, Real timeSinceLastFrame)
        {
            mLast = mCurrent; 
            
            Real sr = 1 / timeSinceLastFrame;
            Real coef = mCutoff * Math::TWO_PI / sr;
            if (coef > 1) 
                coef = 1;
            else if (coef < 0) 
                coef = 0;

            Real feedback = 1 - coef;
           
            return mCurrent = (coef * v) + (feedback * mLast); 
        }

        Real read()
        {
            return mCurrent;
        }
        
};

class Axis 
{
    protected:
        Real mPosition;
        Real mLastPosition;
        Real mLPFPosition;
        Real mButtonThreshold;
        Real mDeadSpot;
        Button mPositiveButton;
        Button mNegativeButton;

        int mFirstPosition;
        bool mFirst;
        bool mMoving;

        LowPassFilter mLPF;
        
    private:
        
    public:
        Axis(Real deadSpot=0.005, Real lpf=5.0) : mLPF(lpf)
        {
            mDeadSpot = deadSpot;
            mPosition = 0.0;
            mButtonThreshold = 0.5;
            mFirst = true;
            mFirstPosition = 0;
            mMoving = false;
        }
        
        void setPosition(Real position, Real timeSinceLastFrame)
        {
            
            mLastPosition = mPosition;
            mPosition = position;

            mLPFPosition = mLPF.write(mPosition,timeSinceLastFrame);
            
            mPositiveButton.setState(mPosition>mButtonThreshold);
            mNegativeButton.setState(mPosition<-mButtonThreshold);
        }

        void setPositionRaw(int position, Real timeSinceLastFrame)
        {
            if(mFirst) 
            {
                mFirstPosition=position;
                mFirst=false;
            }

            if(!mMoving)
            {
                if(position!=mFirstPosition)
                    mMoving=true;
                else
                    position=0;
            }

            setPosition(position/32767.0,timeSinceLastFrame);
            
        }

        bool isDead() {
            return Math::Abs(getPosition())<mDeadSpot;
        }
        
        Real getPosition() {
            return mPosition;
        }

        Real getLPFPosition() {
            return mLPFPosition;
        }

        Real setLPFCutoff(Real freq) {
            mLPF.setCutoff(freq);
        }

        Button &getPositiveButton() {
            return mPositiveButton;
        }
        
        Button &getNegativeButton() {
            return mNegativeButton;
        }
};

class JoystickListener: public FrameListener
{
    protected:
        SDL_Joystick *mJoystick;
        
        int mJoystickNumAxes;
        int mJoystickNumHats;
        int mJoystickNumButtons;

        Button *mButtons;
        Axis *mAxes;
        Axis *mHats;

        Button dummyButton;
        Axis dummyAxis;

        
    private:
        float deadspot(float input, float threshold) {
            if(input>threshold) {
                return input-threshold;
            }
            else if(input<-threshold) {
                return input+threshold;
            }
            else return 0;
            
        }

     
        
        bool setupState() {

            mJoystickNumAxes = SDL_JoystickNumAxes(mJoystick);
            //printf("mJoystickNumAxes %d\n",mJoystickNumAxes);
            mJoystickNumButtons = SDL_JoystickNumButtons(mJoystick);
            //printf("mJoystickNumButtons %d\n",mJoystickNumButtons);
            mJoystickNumHats = SDL_JoystickNumHats(mJoystick);
            //printf("mJoystickNumHats %d\n",mJoystickNumHats);

            if((mButtons = (Button*)malloc(mJoystickNumButtons*sizeof(Button)))==0)
                return false;

            for(int i=0;i<mJoystickNumButtons;i++) {
                mButtons[i] = Button();
            }

            if((mAxes = (Axis*)malloc(mJoystickNumAxes*sizeof(Axis)))==0)
                return false;
            
            if((mHats = (Axis*)malloc(2*mJoystickNumHats*sizeof(Axis)))==0)
                return false;
            
            for(int i=0;i<mJoystickNumAxes;i++) {
                mAxes[i] = Axis();
            }

            for(int i=0;i<mJoystickNumHats*2;i++) {
                mHats[i] = Axis();
            }


        }
        
    public:
        JoystickListener()
        {
            
            mJoystick = 0;
            mJoystickNumAxes = 0;
            mJoystickNumHats = 0;
            mJoystickNumButtons = 0;
    
            int numJoysticks = SDL_NumJoysticks();

            printf("SDL_NumJoysticks==%d\n",numJoysticks);
            
            for(int i=0;i<SDL_NumJoysticks();i++) {
                printf("SDL Joystick %d: \"%s\"\n",i,SDL_JoystickName(i));
                if(!mJoystick) {
                    mJoystick = SDL_JoystickOpen(i);
                    if(mJoystick) {
                        printf("(opened)\n");

                        // FIXME: ignores error
                        setupState();

                    } else {
                        printf("(failed to open joystick)\n");
                    }
                }
            }

        }
       
        Button &getButton(int n)
        {
            if(n<0 || n>=mJoystickNumButtons)
                return dummyButton;

            return mButtons[n];
        }
        
        Axis &getAxis(int n)
        {
            if(n<0)
            {
                return dummyAxis;
            }
            else if(n>=mJoystickNumAxes)
            {
                int m = n-mJoystickNumAxes;
                if(m>=mJoystickNumHats*2)
                {
                    return dummyAxis;
                }
                else
                {
                    return mHats[m];
                }
            }
            else
            {
                return mAxes[n];
            }
        }
        
        bool frameStarted(const FrameEvent &evt) {


            SDL_JoystickUpdate();

            for(int i=0;i<mJoystickNumButtons;i++) {
                mButtons[i].setState(SDL_JoystickGetButton(mJoystick,i)==1);
            }

            for(int i=0;i<mJoystickNumAxes;i++) {
                int pos = SDL_JoystickGetAxis(mJoystick,i);
                //printf("joystick %d, pos %d\n",i,pos);
                mAxes[i].setPositionRaw(pos,evt.timeSinceLastFrame);
            }
            
            for(int i=0;i<mJoystickNumHats;i++) {
                Uint8 pos = SDL_JoystickGetHat(mJoystick,i);
                //printf("joystick %d, pos %d\n",i,pos);
                int xpos = 0;
                int ypos = 0;
                if(pos&SDL_HAT_RIGHT) xpos=32767;
                if(pos&SDL_HAT_LEFT) xpos=-32767;
                if(pos&SDL_HAT_DOWN) ypos=32767;
                if(pos&SDL_HAT_UP) ypos=-32767;
                mHats[i*2].setPositionRaw(xpos,evt.timeSinceLastFrame);
                mHats[(i*2)+1].setPositionRaw(ypos,evt.timeSinceLastFrame);
            }

            return true;
        }

};


#endif
