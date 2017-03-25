#version 330 core
out vec3 color;
in vec3 pos;

void main()
{
/* 
 * Step 1
 *
 * if(gl_FragCoord.y > 240)
 *    color = vec3(0.3, 0.6, 1.0);
 * else
 *    color = fragmentColor;
 *
 * color = fragmentColor;
 * color.x = color.x + (1.0 - color.x) * distance(point, gl_FragCoord) / 1500;
 * color.y = color.y + (1.0 - color.y) * distance(point, gl_FragCoord) / 1500;
 * color.z = color.z + (1.0 - color.z) * distance(point, gl_FragCoord) / 1500;
 * if(distance(point, gl_FragCoord) < 20)
 *    discard;
 */  color = vec3(0, 0, 0);
   if(pos.z > 1)
      discard;
}
