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

#ifndef H_OSCListener
#define H_OSCListener

#include <lo/lo.h>
#include <lo/lo_lowlevel.h>

#define MAX_ENVELOPES 100

struct Envelope
{
    String path;
    lo_message message;
};

class OSCListener: public FrameListener
{

    protected:
        lo_server mLoServer;
        lo_address mTargetAddress;
        //lo_bundle mLoBundle;

        //bool mBundleReady;

        lo_message mEmptyMessage;
        Envelope envelopes[MAX_ENVELOPES];
        int envelopeCount;

        Timer *mTimer;

        const Real mTimeout;

    private:

    public:


        
        OSCListener(uint16 port, String targetURL) : mTimeout(2000)
        {
            mLoServer = lo_server_new_with_proto(StringConverter::toString(port).c_str(),LO_UDP,NULL);
        
            mTargetAddress = lo_address_new_from_url(targetURL.c_str());
            
            addHandlers();

            mEmptyMessage = lo_message_new();

            envelopeCount = 0;

            mTimer = PlatformManager::getSingleton().createTimer();

        }

        /*
        {

            std::pair<String,lo_message> *pair;
            lo_message *lom;

            pair->first = track.mOSCName+"/twistAxis";
            lom = &pair->second;
            
            *lom = lo_message_new();
            lo_message_add_float(*lom,frame.mYaw);
            lo_message_add_float(*lom,frame.mPitch);
            lo_message_add_float(*lom,frame.mRoll);
           
            mvec.push_back(pair);
        }
        */

        void sendMessage(String path, lo_message msg)
        {
            if(envelopeCount<MAX_ENVELOPES)
            {
                Envelope &envelope = envelopes[envelopeCount++];
                envelope.path = path;
                envelope.message = msg;
            }
            else
            {
                fprintf(stderr,"dropping message to %s!\n",path.c_str());
                lo_message_free(msg);
            }
            
            //lo_send_message(mTargetAddress,path.c_str(),msg);
        }
       
        /*
        void sendBundle(String path, lo_bundle b)
        {
            lo_send_bundle(mTargetAddress,b);
        }
        */

        void send(String path)
        {
            lo_message lom;
            lom = lo_message_new();
            sendMessage(path,lom);
            //lo_send_message(mTargetAddress,path.c_str(),mEmptyMessage);
            //lo_send_message(mTargetAddress,path.c_str(),mEmptyMessage);
        }
        
        void send(String path, int i1)
        {
            lo_message lom = lo_message_new();
            lo_message_add_int32(lom,i1);
            sendMessage(path,lom);
            //lo_send_message(mTargetAddress,path.c_str(),lom);
            //lo_message_free(lom);
        }

        // to make the choice unambiguious to the compiler
        void send(String path, double d1)
        {
            this->send(path,(Real)d1);
        }

        void send(String path, Real r1)
        {
            lo_message lom = lo_message_new();
            lo_message_add_float(lom,r1);
            sendMessage(path,lom);
            //lo_send_message(mTargetAddress,path.c_str(),lom);
            //lo_message_free(lom);

        }
        
        void send(String path, Real r1, Real r2)
        {
            lo_message lom = lo_message_new();
            lo_message_add_float(lom,r1);
            lo_message_add_float(lom,r2);
            sendMessage(path,lom);
            //lo_send_message(mTargetAddress,path.c_str(),lom);
            //lo_message_free(lom);
        }

        void send(String path, Real r1, Real r2, Real r3)
        {
            lo_message lom = lo_message_new();
            lo_message_add_float(lom,r1);
            lo_message_add_float(lom,r2);
            lo_message_add_float(lom,r3);
            sendMessage(path,lom);
            //lo_send_message(mTargetAddress,path.c_str(),lom);
            //lo_message_free(lom);
        }
        
        bool frameStarted(const FrameEvent &evt) 
        {
            // process pending messages
            while(lo_server_recv_noblock(mLoServer,0)>0);

            // always return true
            return true;
        }

        bool frameEnded(const FrameEvent &evt)
        {
            if(mTimer->getMilliseconds()<mTimeout) {
                lo_bundle b = lo_bundle_new(LO_TT_IMMEDIATE);
                for(int i=0;i<envelopeCount;i++) {
                    Envelope &envelope = envelopes[i];
                    lo_bundle_add_message(b,envelope.path.c_str(),envelope.message);
                }
                lo_send_bundle(mTargetAddress,b);
                lo_bundle_free(b);
            }

            // delete messages
            for(int i=0;i<envelopeCount;i++) {
                Envelope &envelope = envelopes[i];
                lo_message_free(envelope.message);
            }

            envelopeCount=0;

            return true;
        }
        
    private:

        static int generic_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
        {
            printf("generic_handler path=[%s] types=[%s]\n",path,types);
            return 0;
        }
        
        static int heartbeat_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
        {
            OSCListener *o = (OSCListener*)user_data;
           
            o->mTimer->reset();
            //printf("reset timer\n");
            return 0;
        }

        void addHandlers()
        {
            lo_server_add_method(mLoServer, "/hb", NULL, heartbeat_handler, this);
            lo_server_add_method(mLoServer, NULL, NULL, generic_handler, NULL);
        };
};

#endif
