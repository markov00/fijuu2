fijuu v2 (aka fijuuu)
---------------------
I'm resuming this old project by Julian Oliver and Steven Pickles because I loved it and it will be great to bring it back to life, maybe with some new technologies like WebGL.


developers: 

	julian oliver 
		http://selectparks.net/~julian

	pix (aka steven pickles) 
		http://pix.test.at


contact:

	http://fijuu.com/

	info@fijuu.com


thanks:

	this long awaited update of fijuu was a commission for Cybersonica 06.
		http://cybersonica.org/

licensing:
	
	this program is distributed under the terms of the Gnu General Public License,
	see the file COPYING for details.


requirements:

	hardware (bare minimum requirements)

		2Ghz Pentium-M
		ATI Radeon 9700

		1x Logitech Dual Action gamepad
			any gamepad with at least 6 axes and 9 buttons will work but the
			layout may not be useable.

	software:
	
		Linux (tested with 2.6.13)
			with working direct rendering and sound (alsa or oss)

		OGRE 3D v1.2 RC2
			http://www.ogre3d.org/
			
		Pure-Data (tested with v0.39-2)
			http://crca.ucsd.edu/~msp/software.html
		
			with the following externals:
				dist~
				init
				scale
				t3_line~
				t3_metro
				dumpOSC
				OSCroute
				sendOSC
				spigot~
				
				these are all available at http://sourceforge.net/projects/pure-data
				in the externals folder of cvs. but i think they are also included in 
				most packagings of "pd-extended".
				
		liblo
			http://liblo.sourceforge.net/


instructions:

	building (ideally):

		$ make

	running:

		$ ( cd pd && pd fijuuu.pd ) &
		$ ./fijuuu

pix.
