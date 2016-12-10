///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2003, ATI Technologies, Inc., All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted,
// provided that the above copyright notice appear in all copies and derivative
// works and that both the copyright notice and this permission notice appear in
// support documentation, and that the name of ATI Technologies, Inc. not be used
// in advertising or publicity pertaining to distribution of the software without
// specific, written prior permission.
//
///////////////////////////////////////////////////////////////////////////////

/*
 * GL SL port of RenderMan(R) xray test shader
 *
 * Auto-generated by Ashli for the most part, hand tweaked
 * modified by Jeff Doyle (nfz) July 2004
 */

// vertex to fragment shader io
varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 LightDir;

// globals
uniform float edgefalloff;
uniform vec4 color;

void main()
{
    float opac = dot(normalize(Normal), normalize(EyeDir));
    opac = abs(opac);
    opac = 1.0 - pow(opac, edgefalloff);
    
    gl_FragColor =  opac * color;
    gl_FragColor[3] = opac;
}


