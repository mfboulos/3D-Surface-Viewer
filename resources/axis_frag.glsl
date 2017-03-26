#version 330 core

uniform float ZL;
uniform float ZH;
uniform float temp;

out vec3 color;
in vec3 pos;

void main()
{
   color = vec3(0, 0, 0);
   if((pos.z < ZL || pos.z > ZH) && temp == 0)
      discard;
}
