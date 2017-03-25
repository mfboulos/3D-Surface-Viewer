#version 330 core
out vec3 color;
in vec3 pos;

void main()
{
   color = vec3(.5 + pos.z / 2, .9 + pos.z / 10, 0);
   if(abs(pos.z) > 1)
      discard;
}
