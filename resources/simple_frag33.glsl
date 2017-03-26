#version 330 core

uniform float ZL;
uniform float ZH;

out vec3 color;
in vec3 pos;

void main()
{
   color = vec3(.5 + pos.z / 2, .9 + pos.z / 10, 0);
   if(pos.z < ZL + 0.003 || pos.z > ZH - 0.003)
      color = vec3(0, 0, 0);
   if(pos.z < ZL || pos.z > ZH) 
            discard;
}
