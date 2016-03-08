#include <bits/stdc++.h>
//#include <iostream>
//#include <cmath>
//#include <fstream>
//#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct target {
  float x,y;
  VAO *object;
};
struct obstacle
{
  float x,y;
  VAO *obstacle_object;
};
typedef struct obstacle obstacle;
typedef struct target target;
target target_list[10];
obstacle obstacle_list[5];
GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


 /* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
/**************************
 * Customizable functions *
 **************************/
int state = 0, key = 0, state1;
float v = 12, vx, vy, vxi, vyi, u, ux[5], uy[5]={4,-4,0,0,0};
float radius1 = 0.5;
float PI = 3.142;
float radius = 0.2;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
double cannon_rotation = 45, angle = -90;
float xi = -7.0f, yi = -3.5f, t = 0, x, y, t1=0, xx, yy[2]={-2,2}, t2=0;
int counttot[10]={0}; 
float zoom=1,trans;
std::vector<VAO*> vec[2];
float xp,yp;
int mode;
float lead,centd,segd;
int score;
int co = 0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_RIGHT:
                state = 0;
                
                //cannon_rotation-=5;
                break;
            case GLFW_KEY_LEFT:
                state = 0;
                
                //cannon_rotation+=5;
                break;
            case GLFW_KEY_SPACE:
                x = xi = -7.0f;
                y = yi = -3.5f;
                t = 0;
                u = v;
                angle = cannon_rotation;
                vxi = u*cos(angle*M_PI/180.0f);
                vyi = u*sin(angle*M_PI/180.0f);
                co++;
                break;
            case GLFW_KEY_DOWN:
                state1 = 0;
                break;
            case GLFW_KEY_UP:
                state1 = 0;
                break;

                // do something ..
                break;
            case GLFW_KEY_W:
                zoom+=0.05;
                Matrices.projection = glm::ortho(-8.0f+trans, 8.0f/zoom+trans, -4.5f, 4.5f/zoom, 0.1f, 500.0f);
                break;
            case GLFW_KEY_S:
                if(zoom>1) zoom-=0.05;
                while(8.0f/zoom+trans>8.0) trans--;
                trans++;
                Matrices.projection = glm::ortho(-8.0f+trans, 8.0f/zoom+trans, -4.5f, 4.5f/zoom, 0.1f, 500.0f);
                break;
            case GLFW_KEY_A:
                if(trans>0) trans--;
                Matrices.projection = glm::ortho(-8.0f+trans, 8.0f/zoom+trans, -4.5f, 4.5f/zoom, 0.1f, 500.0f);
                break;
            case GLFW_KEY_D:
                if(8.0f/zoom+trans<8.0) trans++;
                Matrices.projection = glm::ortho(-8.0f+trans, 8.0f/zoom+trans, -4.5f, 4.5f/zoom, 0.1f, 500.0f);
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_RIGHT:
                state = 1;
                break;
            case GLFW_KEY_LEFT:
                state = 2;
                break;
            case GLFW_KEY_UP:
                state1 = 1;
                break;
            case GLFW_KEY_DOWN:
                state1 = 2;
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *square, *cannon, *base, *circle, *circlebase;
void createObstacle ()
{
  GLfloat vertex_buffer_data [] = {
    -0.25,-1.0,0,
    0.25,-1.0,0,
    0.25,1.0,0,
    0.25,1.0,0,
    -0.25,1.0,0,
    -0.25,-1.0,0
  };
   GLfloat color_buffer_data [] = {
    1,0,0,
    0,0,1,
    0,1,0,
    0,1,0,
    0.3,0.3,0.3,
      1,0,0
  };
  for(int i=0;i<2;i++)
  {
    obstacle_list[i].obstacle_object = create3DObject (GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  }
}
// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createSquare()
{
  static const GLfloat vertex_buffer_data [] = {
    -0.5,-0.5,0,
    0.5,-0.5,0,
    0.5,0.5,0,
    0.5,0.5,0,
    -0.5,0.5,0,
    -0.5,-0.5,0
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0,
    0,0,1,
    0,1,0,
    0,1,0,
    0.3,0.3,0.3,
    1,0,0
  };
  square = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createCircle()
{
  float i;
  int j;
  GLfloat vertex_buffer_data [1000];
  for( i = 1,j=0; i <= 72; i++)
  {
    vertex_buffer_data[j++] = cos(i*5) * radius;
    vertex_buffer_data[j++] = sin(i*5) * radius;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = cos((i-1)*5)*radius;
    vertex_buffer_data[j++] = sin((i-1)*5)*radius;
    vertex_buffer_data[j++] = 0.0;
  }
  GLfloat color_buffer_data [1000];
  for(i = 1,j=0;i <=72*3; i++)
  {
    color_buffer_data[j++]= 153.0;
    color_buffer_data[j++]= 76.0;
    color_buffer_data[j++]= 0.0;
  }
  circle = create3DObject(GL_TRIANGLES, 216, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCannon()
{
  static const GLfloat vertex_buffer_data [] ={
    -1.5,-0.5,0,
    1.5,-0.5,0,
    1.5,0.5,0,
    1.5,0.5,0,
    -1.5,0.5,0,
    -1.5,-0.5,0
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0,
    0,0,1,
    0,1,0,
    0,1,0,
    0.3,0.3,0.3,
    1,0,0
  };
  cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBase()
{
  static const GLfloat vertex_buffer_data [] = {
    -8,-0.4,0,
    8,-0.4,0,
    8,0.4,0,
    8,0.4,0,
    -8,0.4,0,
    -8,-0.4,0
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0,
    0,0,1,
    0,1,0,
    0,1,0,
    0.3,0.3,0.3,
    1,0,0
  };
  base = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCircleBase()
{
 float i;
  int j;
  GLfloat vertex_buffer_data [1000];
  for( i = 1,j=0; i <= 72; i++)
  {
    vertex_buffer_data[j++] = cos(i*5) * radius1;
    vertex_buffer_data[j++] = sin(i*5) * radius1;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = cos((i-1)*5)*radius1;
    vertex_buffer_data[j++] = sin((i-1)*5)*radius1;
    vertex_buffer_data[j++] = 0.0;
  }
  GLfloat color_buffer_data [1000];
  for(i = 1,j=0;i <=72*3; i++)
  {
    color_buffer_data[j++]= 0.0;
    color_buffer_data[j++]= 0.3;
    color_buffer_data[j++]= 1.0;
  }
  circlebase = create3DObject(GL_TRIANGLES, 216, vertex_buffer_data, color_buffer_data, GL_FILL); 
}
void createObject ()
{
  int i;
  int j;
  GLfloat vertex_buffer_data [1000];
  for( i = 1,j=0; i <= 72; i++)
  {
    vertex_buffer_data[j++] = cos(i*5) * radius1;
    vertex_buffer_data[j++] = sin(i*5) * radius1;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = 0.0;
    vertex_buffer_data[j++] = cos((i-1)*5)*radius1;
    vertex_buffer_data[j++] = sin((i-1)*5)*radius1;
    vertex_buffer_data[j++] = 0.0;
  }
  GLfloat color_buffer_data [1000];
  for(i = 1,j=0;i <=72*3; i++)
  {
    color_buffer_data[j++]= 0;
    color_buffer_data[j++]= 153;
    color_buffer_data[j++]= 0;
  }

  for(int i=0;i<5;i++)
    target_list[i].object=create3DObject (GL_TRIANGLES, 216, vertex_buffer_data, color_buffer_data, GL_FILL);
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float obstacle1_rotation = 0, obstacle2_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  //Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  /*glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
   triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
  // Matrices.model = glm::mat4(1.0f);
  // MVP = VP * Matrices.model;
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject(square);*/
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannon = glm::translate (glm::vec3(-7.0f, -3.2f, 0.0f));
  glm::mat4 rotateCannon = glm::rotate((float)(cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 translateCannon1 = glm::translate (glm::vec3(1.5f,0.0f,0.0f));
  Matrices.model *= (translateCannon * rotateCannon * translateCannon1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircle = glm::translate (glm::vec3(x, y, 0.0f));
  Matrices.model *= translateCircle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circle);
  t+=0.02;

  
  x=xi+vxi*t;
  y=yi+vyi*t-0.5*9.8*t*t;
    
  if(y>-3.6 && y<-3.4 && x>=xi && t>0)
  {
    
    if(u>2)
      u/=2;
    else
    {
      u=2;
      y=-3.5;
    }
    t=0;
    xi=x;
    yi=y;
    vxi=u*cos(angle*M_PI/180.0f);
    vyi=u*sin(angle*M_PI/180.0f);
    
  }
  
  
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBase = glm::translate (glm::vec3(0.0f,-4.1f,0.0f));
  Matrices.model *= translateBase;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(base);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircleBase = glm::translate (glm::vec3(-7.0f, -3.2f, 0.0f));
  Matrices.model *= translateCircleBase;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circlebase);
  if(state==1)
    cannon_rotation-=1;
  else if(state==2)
    cannon_rotation+=1;
  if(state1==1)
    v+=0.1;
  else if(state1==2)
  {
    if(v<10)
      v=10;
    v-=0.1;

  }
 
  
 
 for(int i=0;i<5;i++)
  {
    Matrices.model = glm::mat4(1.0f); 
    glm::mat4 translateObject;
    translateObject = glm::translate (glm::vec3(target_list[i].x, target_list[i].y, 0.0f));
    Matrices.model *= translateObject;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    //if(target_list[i].x<7.5 && target_list[i].y>-4.0)
    draw3DObject(target_list[i].object);
  }

  Matrices.model = glm::mat4(1.0f);
  //glm::mat4 translateObstacle1 = glm::translate (glm::vec3(-2.5f, -3.0f, 0.0f));
  glm::mat4 rotateObstacle1 = glm::rotate((float)(obstacle1_rotation*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 translateObstacle3 = glm::translate (glm::vec3(-3, yy[0], 0.0f));
  Matrices.model *= ( translateObstacle3 * rotateObstacle1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle_list[0].obstacle_object);

  Matrices.model = glm::mat4(1.0f);
 // glm::mat4 translateObstacle2 = glm::translate (glm::vec3(-1.0f, 3.0f, 0.0f));
  glm::mat4 rotateObstacle2 = glm::rotate((float)(obstacle2_rotation*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 translateObstacle4 = glm::translate (glm::vec3(-1, yy[1], 0.0f));
  Matrices.model *= ( translateObstacle4 * rotateObstacle2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle_list[1].obstacle_object);
  obstacle1_rotation+=2;
  obstacle2_rotation+=2;

  for(int i=0;i<vec[0].size();i++){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate = glm::translate (glm::vec3(0,0,0));        // glTranslatef
    Matrices.model = translate;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(vec[0][i]);
  }
  
  for(int i=0;i<vec[1].size();i++){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate = glm::translate (glm::vec3(0,0,0));        // glTranslatef
    Matrices.model = translate;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(vec[1][i]);
  }



}
 
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

void obstacleMotion ()
{

  if(yy[0]<=-4 && t1>0.5)
  {
    uy[0]=4;
    t1=0;
    obstacle_list[0].y=yy[0];
    //yy[0]=0;
    //cout<<"1\n";
  }
  else if(yy[0]>=4 && t1>0.5)
  {
    uy[0]=-4;
    t1=0;
    obstacle_list[0].y=yy[0];
    //yy[0]=0;
    //cout<<"2\n";
  }
  
  if(yy[1]<=-4 && t2>0.5)
  {
    uy[1]=4;
    t2=0;
    obstacle_list[1].y=yy[1];
    //yy[1]=0;
    //cout<<"3\n";
  }
  else if(yy[1]>=4 && t2>0.5) 
  {
    uy[1]=-4;
    t2=0;
    obstacle_list[1].y=yy[1];
    //yy[1]=0;
    //cout<<"4\n";
  }
  
  //cout<<yy[0]<<" "<<yy[1]<<" "<<uy[0]<<" "<<uy[1]<<" "<<t1<<endl;

  yy[0]=obstacle_list[0].y + uy[0]*t1;
  yy[1]=obstacle_list[1].y + uy[1]*t1;
  t1+=0.005;
  t2+=0.005;
  
}

void detectCollision()
{
  for(int i=0;i<5;i++)
  {
    if(sqrt(pow((x-target_list[i].x),2)+pow((y-target_list[i].y),2))<=0.7)
    {
      //printf("hello %f %f %f %f\n",x,y,target_list[i].x,target_list[i].y);
      vxi/=2;
      vxi*=-1;
      vyi/=3;
      t=0.1;
      xi=x;
      yi=y;
      counttot[i]++;
      score+=50;
      if(score==500)
      {
        cout<<score<<endl;
        glfwTerminate();
        exit(EXIT_SUCCESS);

      }
      //createObject();
      if(counttot[i]==2)
      { 
        target_list[i].x=10;
        target_list[i].y=10;
      }
    }
  }
}

void line(float x1, float y1,float x2, float y2){
        //cout<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
        GLfloat vertex_buffer_data [] = {
                x1,y1,0,
                x1,y1,0, 
                x2,y2,0, 
        };
        static const GLfloat color_buffer_data [] = {
        0.85,0.85,0.85,
        0.85,0.85,0.85,
        0.85,0.85,0.85,
};
        vec[mode].push_back(create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE));
}

void drawSubDigit(int i){
        if(i==1)
                line(xp,yp+1.5*segd,xp,yp);
        if(i==2)
                line(xp,yp,xp,yp-1.5*segd);
        if(i==3)
                line(xp-segd,yp+1.5*segd,xp-segd,yp);
        if(i==4)
                line(xp-segd,yp,xp-segd,yp-1.5*segd);
        if(i==5)
                line(xp,yp+1.5*segd,xp-segd,yp+1.5*segd);
        if(i==6)
                line(xp,yp,xp-segd,yp);
        if(i==7)
                line(xp,yp-1.5*segd,xp-segd,yp-1.5*segd);
}

void drawDigit(int digit){
        if(digit==-1){
                drawSubDigit(6);
        }

        if(digit==0){
                drawSubDigit(1);
                drawSubDigit(2);
                drawSubDigit(3);
                drawSubDigit(4);
                drawSubDigit(5);
                drawSubDigit(7);
        }

        if(digit==1){
                drawSubDigit(1);
                drawSubDigit(2);
        }

        if(digit==2){
                drawSubDigit(5);
                drawSubDigit(1);
                drawSubDigit(6);
                drawSubDigit(4);
                drawSubDigit(7);
        }

        if(digit==3){
                drawSubDigit(5);
                drawSubDigit(1);
                drawSubDigit(6);
                drawSubDigit(2);
                drawSubDigit(7);
        }

        if(digit==4){
                drawSubDigit(3);
                drawSubDigit(1);
                drawSubDigit(6);
                drawSubDigit(2);
        }

        if(digit==5){
                drawSubDigit(3);
                drawSubDigit(5);
                drawSubDigit(6);
                drawSubDigit(2);
                drawSubDigit(7);
        }

        if(digit==6){
                drawSubDigit(3);
                drawSubDigit(5);
                drawSubDigit(6);
                drawSubDigit(2);
                drawSubDigit(7);
                drawSubDigit(4);
        }

        if(digit==7){
                drawSubDigit(1);
                drawSubDigit(5);
                drawSubDigit(2);
        }

        if(digit==8){
                drawSubDigit(3);
                drawSubDigit(4);
                drawSubDigit(5);
                drawSubDigit(6);
                drawSubDigit(2);
                drawSubDigit(7);
                drawSubDigit(1);
        }

        if(digit==9){
                drawSubDigit(3);
                drawSubDigit(5);
                drawSubDigit(6);
                drawSubDigit(2);
                drawSubDigit(7);
                drawSubDigit(1);
        }
}

void getScore(int value){
        xp=-6+trans;
        yp=3.5/zoom;
        mode=0;
        centd= 0.2/zoom;
        segd = 0.1 /zoom;
        int pvalue=value;
        xp-=0.5*centd+(float)1/zoom;
        if(value==0){
                drawDigit(0);
                xp-=(centd);
        }
        while (value!=0){
                drawDigit(value%10);
                xp-=(centd);
                value = value/10;
        }
        if(pvalue<10){
                drawDigit(0);
                xp-=(centd);
        }
        if(pvalue<100)
                drawDigit(0);
        yp=4/zoom;xp=-7.5+trans;
        drawDigit(5);
        xp+=centd;
        drawSubDigit(3);
        drawSubDigit(4);
        drawSubDigit(5);
        drawSubDigit(7);
        xp+=centd;
        drawDigit(0);
        xp+=centd;
        drawSubDigit(1);
        drawSubDigit(2);
        drawSubDigit(3);
        drawSubDigit(4);
        drawSubDigit(5);
        drawSubDigit(6);
        xp+=centd;
        drawSubDigit(3);
        drawSubDigit(4);
        drawSubDigit(5);
        drawSubDigit(6);
        drawSubDigit(7);
}


void detectCollision1 ()
{

  if(sqrt(pow(x+3,2)+pow((y-yy[0]),2))<=1.2)
  {
    vxi/=2;
    vxi*=-1;
    vyi/=3;
    t=0.1;
    xi=x;
    yi=y;
  //  cout<<x<<" "<<y<<" "<<yy[0]<<" "<<yy[1]<<endl;
  }

  if(sqrt(pow(x+1,2)+pow((y-yy[1]),2))<=1.2)
  {
    vxi/=2;
    vxi*=-1;
    vyi/=3;
    t=0.1;
    xi=x;
    yi=y;
    //cout<<x<<" "<<y<<" "<<yy[0]<<" "<<yy[1]<<endl;
  }

  
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
  createSquare ();
  createCannon ();
  createCircle ();
  createBase ();
  createCircleBase ();
  
  for(int i=0;i<5;i++)
  {

    
    target_list[i].x =  rand()%8;
  //  cout << target_list[i].x << endl;
    target_list[i].y = rand()%8-3;
  }
  obstacle_list[0].y = 0;
  obstacle_list[1].y = 0; 

  createObject ();
  createObstacle ();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1600;
	int height = 900;
  double xpos, ypos;

    GLFWwindow* window = initGLFW(width, height);

	   initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        detectCollision();

        obstacleMotion ();

        detectCollision1 ();

        //cout<<angle<<endl;
        glfwGetCursorPos (window, &xpos, &ypos);

        glfwGetFramebufferSize(window, &width, &height);
        if(state==0)
        {

          xpos=-77+(float)154.0/width*xpos;

          ypos=-40+(float)80.0/height*ypos;

          ypos*=-1;

          float x = -70.0;

          float y = -5.0;
        
          cannon_rotation = atan2 (ypos-y,xpos-x) * 180 / M_PI;
        }
        if(cannon_rotation<0)
          cannon_rotation=0;
        if(cannon_rotation>90)
          cannon_rotation=90;
        //cout<<xpos<<" "<<ypos<<endl;
        vec[1].clear();
        vec[0].clear();
        getScore(score);

        if(co==15)
        {
          cout<<score<<endl;
          glfwTerminate();
          exit(EXIT_SUCCESS);
          
        }


        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
          

            // do something every 0.5 seconds ..
          



          last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
