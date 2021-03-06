/*
   ZJ Wood CPE 471 Lab 3 base code - includes use of glee
https://github.com/nshkurkin/glee
 */

#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Open source function parsing code
// Found at URL: http://www.partow.net/programming/exprtk/index.html
#include "exprtk.hpp"

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

using namespace std;
using namespace glm;

// various global parameters
int pixW, pixH;
int mousePress;
int colorMode;
float res;
float xMin;
float xMax;
float yMin;
float yMax;
float zMin;
float zMax;
float zLow;
float zHigh;
float varX;
float varY;
float xROT;
float yROT;
float xRotBASE;
float yRotBASE;
float scaleVar;
float xBASE;
float yBASE;
float panX;
float panY;
float *scrollVal = NULL;
std::string expr_string;

// type definitions for readability
typedef exprtk::symbol_table<float> symbol_table_t;
typedef exprtk::expression<float>     expression_t;
typedef exprtk::parser<float>             parser_t;

// global variables for function parsing
symbol_table_t symbol_table;
expression_t expression;
parser_t parser;

static const float e_const  = 2.7182818284590452353602874713526624977; // constant e
static const float pi_const = 3.1415926535897932384626433832795028841; // constant pi

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog; //our shader program for the surface
shared_ptr<Program> prog2; //our shader program for the axes

/* Global data associated with triangle geometry - this will likely vary
   in later programs - so is left explicit for now  */
GLuint VertexArrayID;
static GLfloat g_vertex_buffer_data[180045] = {0.0f};

//data necessary to give our triangle data to OGL
GLuint vertexbuffer; 

static void inputPoint(int *index, expression_t expression);
static void inputPoints(int *index, expression_t expression, int i, int j);
static void resizeVals();

static void error_callback(int error, const char *description)
{
   cerr << description << endl;
}

/*
 * Prints function details for the surface displayed
 */
void printFunctionDetails()
{
   cout << "\nFunction: " << expr_string;
   if(zLow != -1 || zHigh != 1)
      cout << " from z = " << ((zLow + 1)/2*(zMax - zMin) + zMin) << " to z = " << ((zHigh + 1)/2*(zMax - zMin));
   cout << "\nResolution: " << (int)floor(res);
   cout << "\nDomain: [" << xMin << ", " << xMax << "] x [" << yMin << ", " << yMax << "]";
   cout << "\nScroll: "; 
   if(scrollVal == &scaleVar)
      cout << "Scale\n";
   else if(scrollVal == &res)
      cout << "Resolution\n";
   else if(scrollVal == &zLow)
      cout << "Lower Z\n";
   else if(scrollVal == &zHigh)
      cout << "Upper Z\n";
   else if(scrollVal == NULL)
      cout << "None\n";
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      cout << "\nSee you next time!\n\n";
      glfwSetWindowShouldClose(window, GL_TRUE);
   } else if(key == GLFW_KEY_C && action == GLFW_PRESS) {
      colorMode += 1;
   } else if(key == GLFW_KEY_P && action == GLFW_PRESS) {
      printFunctionDetails();
   } else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
      if(scrollVal == &scaleVar)
         scrollVal = NULL;
      else
         scrollVal = &scaleVar;
      printFunctionDetails();
   } else if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
      if(scrollVal == &res)
         scrollVal = NULL;
      else
         scrollVal = &res;
      printFunctionDetails();
   } else if(key == GLFW_KEY_L && action == GLFW_PRESS) {
      if(scrollVal == &zLow)
         scrollVal = NULL;
      else
         scrollVal = &zLow;
      printFunctionDetails();
   } else if(key == GLFW_KEY_H && action == GLFW_PRESS) {
      if(scrollVal == &zHigh)
         scrollVal = NULL;
      else
         scrollVal = &zHigh;
      printFunctionDetails();
   } else if(key == GLFW_KEY_X && action == GLFW_PRESS) {
      std::string temp;
      cout << "\nEnter the new minimum X value for the domain: ";
      std::getline(std::cin, temp);
      float temp1 = std::stof(temp, NULL);
      cout << "\nEnter the new maximum X value for the domain: ";
      std::getline(std::cin, temp);
      float temp2 = std::stof(temp, NULL);
      if(temp1 < temp2)
      {
         xMin = temp1;
         xMax = temp2;
      }
      else
         cout << "\nError with bounds, requires xMin < xMax\n";
      printFunctionDetails();
   } else if(key == GLFW_KEY_Y && action == GLFW_PRESS) {
      std::string temp;
      cout << "\nEnter the new minimum Y value for the domain: ";
      std::getline(std::cin, temp);
      float temp1 = std::stof(temp, NULL);
      cout << "\nEnter the new maximum Y value for the domain: ";
      std::getline(std::cin, temp);
      float temp2 = std::stof(temp, NULL);
      if(temp1 < temp2)
      {   
         yMin = temp1;
         yMax = temp2;
      }   
      else
         cout << "\nError with bounds, requires yMin < yMax\n";
      printFunctionDetails();
   } else if(key == GLFW_KEY_UP) {
      if(action == GLFW_PRESS)
         panY -= 0.15;
      else if(action == GLFW_REPEAT)
         panY -= 0.15;
   } else if(key == GLFW_KEY_DOWN) {
      if(action == GLFW_PRESS)
         panY += 0.15; 
      else if(action == GLFW_REPEAT)
         panY += 0.15;
   } else if(key == GLFW_KEY_RIGHT) {
      if(action == GLFW_PRESS)
         panX -= 0.15; 
      else if(action == GLFW_REPEAT)
         panX -= 0.15;
   } else if(key == GLFW_KEY_LEFT) {
      if(action == GLFW_PRESS)
         panX += 0.15; 
      else if(action == GLFW_REPEAT)
         panX += 0.15;
   } else if(key == GLFW_KEY_F && action == GLFW_PRESS) {
      cout << "\nEnter the new function: ";
      std::getline(std::cin, expr_string);
      parser.compile(expr_string, expression);
      zLow = -1;
      zHigh = 1;
      scrollVal = &scaleVar;
      printFunctionDetails();
   }
}

//helper function, toggles a switch to handle view rotation
void MouseButton(int button, int action)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      mousePress = true;
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
   {
      mousePress = false;
      xRotBASE = xROT;
      yRotBASE = yROT;
   }
}

//callback for cursorPos
static void cursor_callback(GLFWwindow *window, double posX, double posY)
{
   if(mousePress)
   {
      glfwGetCursorPos(window, &posX, &posY);
      xROT = xRotBASE + pi_const / 2 * (((4 * posX - 2 * pixW) / pixH) - xBASE);
      yROT = yRotBASE + pi_const / 3 * ((-(4 * posY - 2 * pixH) / pixH) - yBASE);
   }
}

//callback for the mouse when clicked start rotating camera view
static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
   double posX, posY;
   MouseButton(button, action);
   if (mousePress) {
      glfwGetCursorPos(window, &posX, &posY);	
      xBASE = (4 * posX - 2 * pixW) / pixH;
      yBASE = -(4 * posY - 2 * pixH) / pixH;
   }
}

//callback for scroll functionality
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
   if(scrollVal != NULL)
   {
      if(scrollVal == &res || scrollVal == &scaleVar) {
         if(*scrollVal + yoffset/3 < 2)
            *scrollVal = 2;
         else if(*scrollVal + yoffset/3 > 100)
            *scrollVal = 100;
         else
            *scrollVal += yoffset/3;
      } else if(scrollVal == &zLow || scrollVal == &zHigh) {
         if(zLow > zHigh)
         {
            if(scrollVal == &zLow)
               zLow = zHigh;
            else
               zHigh = zLow;
         }
         else if(*scrollVal + yoffset/420 < -1)
            *scrollVal = -1;
         else if(*scrollVal + yoffset/420 > 1)
            *scrollVal = 1;
         else
            *scrollVal += yoffset/420;
      }
   }
}

//if the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int in_width, int in_height) {
   //get the window size - may be different then pixels for retina	
   glfwGetFramebufferSize(window, &pixW, &pixH);
   glViewport(0, 0, pixW, pixH);
}

/*Note that any gl calls must always happen after a GL state is initialized */
static void initGeom() {

   //generate the VAO
   glGenVertexArrays(1, &VertexArrayID);
   glBindVertexArray(VertexArrayID);
   //generate vertex buffer to hand off to OGL
   glGenBuffers(1, &vertexbuffer);
   //set the current state to focus on our vertex buffer
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

   //actually memcopy the data - only do this once
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), (const GLvoid*) g_vertex_buffer_data, GL_DYNAMIC_DRAW);
}

/*
 * Main function for buffer updating
 * Fills buffer data with triangles that construct the surface
 * Adds data for the axes at the end of the buffer
 */
void fillBuffer()
{
   zMin = FLT_MAX;
   zMax = FLT_MIN;
   int index = 0;
   for(int i = 0; i < floor(res); i++)
   {
      for(int j = 0; j < floor(res); j++)
      {
         inputPoints(&index, expression, i, j);
      }
   }

   if(zMin == zMax)
   {
      zMin -= 0.01;
      zMax += 0.01;
   }
      
   // Endpoints of axes
   g_vertex_buffer_data[180001] = fmin(0, fmin(yMin, -yMax));
   g_vertex_buffer_data[180004] = fmax(0, fmax(-yMin, yMax));

   g_vertex_buffer_data[180006] = fmin(0, fmin(xMin, -xMax));
   g_vertex_buffer_data[180009] = fmax(0, fmax(-xMin, xMax));

   g_vertex_buffer_data[180014] = fmin(0, fmin(zMin, -zMax));
   g_vertex_buffer_data[180017] = fmax(0, fmax(-zMin, zMax));

   // Arrows to represent positive direction
   g_vertex_buffer_data[180019] = g_vertex_buffer_data[180004];
   g_vertex_buffer_data[180027] = g_vertex_buffer_data[180009];
   g_vertex_buffer_data[180038] = g_vertex_buffer_data[180017];

   resizeVals();

   g_vertex_buffer_data[180021] = g_vertex_buffer_data[180018] - 1/20.0;
   g_vertex_buffer_data[180022] = g_vertex_buffer_data[180019] - 1/10.0;
   g_vertex_buffer_data[180024] = g_vertex_buffer_data[180018] + 1/20.0;
   g_vertex_buffer_data[180025] = g_vertex_buffer_data[180019] - 1/10.0;
   g_vertex_buffer_data[180030] = g_vertex_buffer_data[180027] - 1/10.0;
   g_vertex_buffer_data[180031] = g_vertex_buffer_data[180028] - 1/20.0;
   g_vertex_buffer_data[180033] = g_vertex_buffer_data[180027] - 1/10.0;
   g_vertex_buffer_data[180034] = g_vertex_buffer_data[180028] + 1/20.0;
   g_vertex_buffer_data[180039] = g_vertex_buffer_data[180036] - 1/30.0;
   g_vertex_buffer_data[180040] = g_vertex_buffer_data[180037] + 1/30.0;
   g_vertex_buffer_data[180041] = g_vertex_buffer_data[180038] - 1/15.0;
   g_vertex_buffer_data[180042] = g_vertex_buffer_data[180036] + 1/30.0;
   g_vertex_buffer_data[180043] = g_vertex_buffer_data[180037] - 1/30.0;
   g_vertex_buffer_data[180044] = g_vertex_buffer_data[180038] - 1/15.0;
}

//General OGL initialization - set OGL state here
static void init()
{
   // Print initial information for OpenGL, surface plotter, etc.
   GLSL::checkVersion();
   cout << "\n**** OpenGL 3D Surface Viewer ****\nMade by Michael Boulos\n";
   cout << "\nControls:\n";
   cout << " * Mouse click/drag - Rotate model view\n";
   cout << " * Arrow keys - Pan model view\n";
   cout << " * [C] - Change color mode (mesh, surface, or both)\n";
   cout << " * [S] - Set mouse scroll to scale (default)\n";
   cout << " * [Q] - Set mouse scroll to resolution\n";
   cout << " * [L] - Set mouse scroll to lower z cutoff\n";
   cout << " * [H] - Set mouse scroll to upper z cutoff\n";
   cout << " * [F] - Define new function in terminal\n";
   cout << " * [X] - Set X parameters in terminal\n";
   cout << " * [Y] - Set Y parameters in terminal\n";
   cout << " * [P] - Print function details\n";
   cout << " * [ESC] - Close the display window\n";

   // Set various params
   mousePress = 0;
   colorMode = 0;
   xBASE = 0;
   yBASE = 0;
   xROT = 0;
   yROT = 0;
   xRotBASE = 0;
   yRotBASE = 0;
   res = 10;
   xMin = -10;
   xMax = 10;
   yMin = -10;
   yMax = 10;
   zLow = -1;
   zHigh = 1;
   panX = 0;
   panY = 1;
   scaleVar = 15;
   scrollVal = &scaleVar;

   // Set background color.
   glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

   // Enable z-buffer test.
   glEnable(GL_DEPTH_TEST);

   // Initialize the GLSL programs.
   prog = make_shared<Program>();
   prog->setVerbose(true);
   prog->setShaderNames(RESOURCE_DIR + "simple_vert33.glsl", RESOURCE_DIR + "simple_frag33.glsl");
   prog->init();
   prog->addUniform("P");
   prog->addUniform("MV");
   prog->addUniform("ZL");
   prog->addUniform("ZH");
   prog->addAttribute("vertPos");

   prog2 = make_shared<Program>();
   prog2->setVerbose(true);
   prog2->setShaderNames(RESOURCE_DIR + "simple_vert33.glsl", RESOURCE_DIR + "axis_frag.glsl");
   prog2->init();
   prog2->addUniform("P");
   prog2->addUniform("MV");
   prog2->addUniform("ZL");
   prog2->addUniform("ZH");
   prog2->addUniform("temp");
   prog2->addAttribute("vertPos");

   // Initialize the functional expression
   varX = 0;
   varY = 0;

   expr_string = "x*x + y*y";

   symbol_table.add_variable("x", varX);
   symbol_table.add_variable("y", varY);
   symbol_table.add_constants(); //adds pi, epsilon, and infinity as constants
   symbol_table.add_constant("e", e_const); //adds e as a constant

   expression.register_symbol_table(symbol_table);

   parser.compile(expr_string, expression);

   // Initial print of function details
   printFunctionDetails();
}

/*
 * Function that updates a single point in the buffer array
 * Updates zMin and zMax based on resulting values
 */
static void inputPoint(int *index, expression_t expression)
{
   g_vertex_buffer_data[(*index)++] = varX;
   g_vertex_buffer_data[(*index)++] = varY;
   g_vertex_buffer_data[(*index)++] = expression.value();
   if(expression.value() != std::numeric_limits<float>::infinity() && expression.value() != -(std::numeric_limits<float>::infinity()))
   {
      if(expression.value() > zMax)
         zMax = expression.value();
      if(expression.value() < zMin)
         zMin = expression.value();
   }
}


// Calls inputPoint to input triangle data into the buffer array
static void inputPoints(int *index, expression_t expression, int i, int j)
{
   varX = i * (xMax - xMin) / floor(res) + xMin;
   varY = j * (yMax - yMin) / floor(res) + yMin;
   inputPoint(index, expression);

   varX = (i + 1) * (xMax - xMin) / floor(res) + xMin;
   inputPoint(index, expression);

   varY = (j + 1) * (yMax - yMin) / floor(res) + yMin;
   inputPoint(index, expression);

   inputPoint(index, expression);

   varX = i * (xMax - xMin) / floor(res) + xMin;
   inputPoint(index, expression);

   varY = j * (yMax - yMin) / floor(res) + yMin;
   inputPoint(index, expression);
}

// Resizes the values in the buffer array based on the mins and maxes of each dimension
static void resizeVals()
{
   for(int i = 0; i < 6 * floor(res) * floor(res); i++)
   {
      g_vertex_buffer_data[3*i] -= xMin;
      g_vertex_buffer_data[3*i] /= ((xMax - xMin) / 2);
      g_vertex_buffer_data[3*i] -= 1;

      g_vertex_buffer_data[3*i + 1] -= yMin;
      g_vertex_buffer_data[3*i + 1] /= ((yMax - yMin) / 2);
      g_vertex_buffer_data[3*i + 1] -= 1;

      g_vertex_buffer_data[3*i + 2] -= zMin;
      g_vertex_buffer_data[3*i + 2] /= ((zMax - zMin) / 2);
      g_vertex_buffer_data[3*i + 2] -= 1;
   }
   for(int i = 0; i < 15; i++)
   {
      g_vertex_buffer_data[180000 + 3*i] -= xMin;
      g_vertex_buffer_data[180000 + 3*i] /= ((xMax - xMin) / 2); 
      g_vertex_buffer_data[180000 + 3*i] -= 1;

      g_vertex_buffer_data[180000 + 3*i + 1] -= yMin;
      g_vertex_buffer_data[180000 + 3*i + 1] /= ((yMax - yMin) / 2); 
      g_vertex_buffer_data[180000 + 3*i + 1] -= 1;

      g_vertex_buffer_data[180000 + 3*i + 2] -= zMin;
      g_vertex_buffer_data[180000 + 3*i + 2] /= ((zMax - zMin) / 2); 
      g_vertex_buffer_data[180000 + 3*i + 2] -= 1;
   }
}

/****DRAW
  This is the most important function in your program - this is where you
  will actually issue the commands to draw any geometry you have set up to
  draw
 ********/
static void render()
{
   // Get current frame buffer size.
   int width, height;
   glfwGetFramebufferSize(window, &width, &height);
   float aspect = width/(float)height;
   glViewport(0, 0, width, height);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Create the matrix stacks - please leave these alone for now
   auto P = make_shared<MatrixStack>();
   auto MV = make_shared<MatrixStack>();

   // Apply orthographic projection.
   P->pushMatrix();
   if (width > height) {
      P->ortho(-2*aspect, 2*aspect, -2, 2, -2, 100.0f);
   } else {
      P->ortho(-2, 2, -2*1/aspect, 2*1/aspect, -2, 100.0f);
   }
   MV->pushMatrix();

   // Change model view based on params.
   MV->scale(vec3(scaleVar/15, scaleVar/15, scaleVar/15));
   MV->translate(vec3(panX, panY, 0));
   MV->rotate(-yROT, vec3(1, 0, 0));
   MV->rotate(-pi_const/3, vec3(1, 0, 0));
   MV->rotate(pi_const/3, vec3(0, 0, 1));
   MV->translate(vec3(.34, .2, -1.2));
   MV->rotate(xROT, vec3(0, 0, 1));

   // Update the buffer
   fillBuffer();
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), (const GLvoid*) g_vertex_buffer_data, GL_DYNAMIC_DRAW);

   prog2->bind();

   float temp = 0;
   // Send the matrices to the shaders
   glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
   glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));

   // Send range restrictions to the shaders
   glUniform1f(prog2->getUniform("ZL"), zLow);
   glUniform1f(prog2->getUniform("ZH"), zHigh);

   // Set up vertex array
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

   // Draw the mesh of the surface
   if(colorMode%3 == 0 || colorMode%3 == 2)
   {
      glUniform1f(prog2->getUniform("temp"), temp);
      for(int i = 0; i < 2 * floor(res) * floor(res); i++)
      {
         glDrawArrays(GL_LINES, 3*i, 2);
      glDrawArrays(GL_LINES, 3*i + 1, 2);
      }
   }

   // Set max distance from axes such that they are displayed
   float X = (xMax - xMin) / 5;
   float Y = (yMax - yMin) / 5;
   float Z = (zMax - zMin) / 5;

   // Draw axes
   temp = 1;
   glUniform1f(prog2->getUniform("temp"), temp);
   if(xMin <= X && xMax >= -X && zMin <= Z && zMax >= -Z)
   {
      glDrawArrays(GL_LINES, 60000, 2);
      glDrawArrays(GL_TRIANGLES, 60006, 3);
   }
   if(yMin <= Y && yMax >= -Y && zMin <= Z && zMax >= -Z)
   {
      glDrawArrays(GL_LINES, 60002, 2);
      glDrawArrays(GL_TRIANGLES, 60009, 3);
   }
   if(xMin <= X && xMax >= -X && yMin <= Y && yMax >= -Y)
   {
      glDrawArrays(GL_LINES, 60004, 2);
      glDrawArrays(GL_TRIANGLES, 60012, 3);
   }

   glDisableVertexAttribArray(0);

   prog2->unbind();

   prog->bind();

   // Send the matrices to the shaders
   glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
   glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));

   // Send range restrictions to shaders
   glUniform1f(prog->getUniform("ZL"), zLow);
   glUniform1f(prog->getUniform("ZH"), zHigh);

   // Set up the vertex array
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

   // Draw the surface
   if(colorMode%3 == 0 || colorMode%3 == 1)
   {
      for(int i = 0; i < 2 * floor(res) * floor(res); i++)
         glDrawArrays(GL_TRIANGLES, 3*i, 3);
   }

   glDisableVertexAttribArray(0);

   prog->unbind();

   // Pop matrix stacks.
   MV->popMatrix();
   P->popMatrix();
}

int main(int argc, char **argv)
{
   if(argc < 2) {
      cout << "Please specify the resource directory." << endl;
      return 0;
   }
   RESOURCE_DIR = argv[1] + string("/");

   /* your main will always include a similar set up to establish your window
      and GL context, etc. */
   // Set error callback as openGL will report errors but they need a call back
   glfwSetErrorCallback(error_callback);

   // Initialize the library.
   if(!glfwInit()) {
      return -1;
   }

   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

   pixW = 640;
   pixH = 480;

   // Create a windowed mode window and its OpenGL context.
   window = glfwCreateWindow(pixW, pixH, "3DPlot", NULL, NULL);
   if(!window) {
      glfwTerminate();
      return -1;
   }
   // Make the window's context current.
   glfwMakeContextCurrent(window);

   // Initialize GLEW.
   glewExperimental = true;
   if(glewInit() != GLEW_OK) {
      cerr << "Failed to initialize GLEW" << endl;
      return -1;
   }
   glGetError();
   cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

   // Set vsync.
   glfwSwapInterval(1);
   // Set keyboard callback.
   glfwSetKeyCallback(window, key_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   //set the cursorPos callback
   glfwSetCursorPosCallback(window, cursor_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);
   //set the scroll callback
   glfwSetScrollCallback(window, scroll_callback);

   /* This is the code that will likely change program to program as you
      may need to initialize or set up different data and state */
   // Initialize scene.
   init();
   initGeom();

   // Loop until the user closes the window.
   while(!glfwWindowShouldClose(window)) {
      // Render scene.
      render();
      // Swap front and back buffers.
      glfwSwapBuffers(window);
      // Poll for and process events.
      glfwPollEvents();
   }

   // Quit program.
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}
