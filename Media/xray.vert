
//
// Jeff Doyle (nfz) July 2004
//

// note: all vectors in object space
varying vec3 Normal;
varying vec3 EyeDir;

uniform vec3 eyePosition;

void main(void) 
{
    gl_Position = ftransform();
    EyeDir      = normalize(eyePosition - gl_Vertex.xyz);
    Normal      = gl_Normal;
  
}
