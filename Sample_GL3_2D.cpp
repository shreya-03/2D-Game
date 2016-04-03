#include <iostream>
#include <cmath>
#include<stdio.h>
#include <fstream>
#include <vector>
#include<stdlib.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include<unistd.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<time.h>
#include<math.h>
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

typedef struct Objects {
	float cx;
	float cy;
	float rad;
} Objects;
GLuint programID;
Objects objects[3];

/* Function to load Shaders - Use it as it is */
VAO *createCircle(float rad,float cx,float cy);
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

// Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
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
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
float rot_angle=0;
float ball_angle=0;
float ball_speed=1;
float slope1,slope2,ball_speed_x,ball_speed_y;
float ball_x;
float ball_y;
float centre_x,centre_y;
float end_t=0;
float rec_init_x=-4.15;
float rec_init_y=-4.3;
float tri_init_x=-4.3;
float tri_init_y=-3.367;
float circle_init_x=-4.3;
float circle_init_y=-2.5;
float arrow_init_x=-4.3;
float arrow_init_y=-2.5;
float add_x=0;
float add_y=0;
float k=2,m=1,g=10,e=0.5;
float zoom=1;
int score=0;
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}
VAO *ball;
//VAO *objects[];
bool flag=false;
bool zoom_flag=false;
bool pan_flag=false;
float time_flight=0;
float angle=0.0,lx=0.0f,lz=-1.0f,x=0.0f,z=5.0f;
/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'e':
        case 'E':
  	    add_x+=0.1;
	    rec_init_x+=add_x;
	    tri_init_x+=add_x;
	    circle_init_x+=add_x;
	    arrow_init_x+=add_x;
	    break;
        case 'W':
        case 'w':
            add_x-=0.1;
	    rec_init_x+=add_x;
	    tri_init_x+=add_x;
	    circle_init_x+=add_x;
            arrow_init_x+=add_x;
	    break;
        case 'b':
	case 'B':
            rot_angle-=5;
	    ball_angle-=5;
            break;
	case 'a':
	case 'A':
	    rot_angle+=5;
	    ball_angle+=5;
	    break;
	case 'f':
	    ball_speed+=0.1;
	    end_t=0;
	    break;
	case 's':
	    ball_speed-=0.1;
	    end_t=0;
	    break;
	case ' ':
	    ball_x=circle_init_x+(1.0*cos(rot_angle*M_PI/180.0f));
	    ball_y=circle_init_y+(1.0*sin(rot_angle*M_PI/180.0f));
	    flag=true;
	    centre_x=ball_x;
	    centre_y=ball_y;
	    ball_angle=rot_angle;
            break;
//        case 
	default:
            break;
    }
}
float pan=0;
/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
	float fraction=0.1f;
	switch(key){
		case GLUT_KEY_LEFT:
//			angle+=0.1f;
//			lx=sin(angle);
//			lz=-cos(angle);
			pan+=0.1f;
			break;
		case GLUT_KEY_RIGHT:
//			angle-=0.1f;
//			lx=sin(angle);
//			lz=-cos(angle);
			pan-=0.1f;
			break;
		case GLUT_KEY_UP:
//			x-=lx*fraction;
//			z-=lz*fraction;
			zoom_flag=true;
			zoom-=0.5;
			break;
		case GLUT_KEY_DOWN:
//			x+=lx*fraction;
//			z+=lz*fraction;
			zoom_flag=true;
			zoom+=0.5;
			break;
	}

//	glutPostRedisplay();
}
bool isDragging=0;
float deltaAngle,xDragStart;
char s[10];
float speed_x;
//const int font=((int)GLUT_BITMAP_9_BY_15;
/* Executed when a special key is released */
void mouseMove (int x, int y)
{
	if(isDragging)
	{
		deltaAngle=(x-xDragStart)*0.0005;
//		lx=sin(angle+deltaAngle);
//		lz=-cos(angle+deltaAngle);
		pan+=sin(deltaAngle);		
	}
}
void scrollMouse(int x,int y)
{
	if(isDragging)
		zoom+=float(y)/4;
	if(zoom<0)
		zoom=0;
}
/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
	    {    
//		    angle+=0.1f;
//		    lx=sin(angle);
//		    lz=-cos(angle);
		    pan+=0.1f;
		    isDragging=1;
		    xDragStart=x;
	    }
	    else
		    isDragging=0;
	break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_DOWN) {
//                angle-=0.1f;
//		lx=sin(angle);
//		lz=-cos(angle);
		pan-=0.1f;
		isDragging=1;
		xDragStart=x;
            }
	    else
		    isDragging=0;
            break;
        default:
            break;
    }
}

// Executed when the mouse moves to position ('x', 'y') 
void mouseMotion (int x, int y)
{
	if(isDragging!=1)
		rot_angle=atan((y-circle_init_y)/(x-circle_init_x))*180.0f/M_PI;
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
//    Matrices.projection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, 0.1f, 500.0f);
}

VAO *triangle,*circle,*rectangle,*arrow,*ground,*obstacle_circle1,*obstacle_tri,*obstacle_circle2;
VAO *bird,*grass1,*grass2,*grass3,*grass4;
VAO* cloud,*cloud1;
VAO* createOval(float radx,float rady,float cx,float cy,float red,float green,float blue);
VAO* createTriangle (float x1,float y1,float x2,float y2,float x3,float y3,float red,float green,float blue)
{
  static const GLfloat vertex_buffer_data [] = {
    x1,y1,0, // vertex 0
    x2,y2,0, // vertex 1
    x3,y3,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    red,green,blue, // color 0
    red,green,blue, // color 1
    red,green,blue, // color 2
  };
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createRectangle (float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float red,float green,float blue)
{
	const GLfloat vertex_buffer_data[]={
		x1,y1,0,//vertex 1
		x2,y2,0,//vertex 2
		x3,y3,0,//vertex 3

		x3,y3,0,//vertex 3
		x4,y4,0,//vertex 4
		x1,y1,0,//vertex 1
	};
	return create3DObject(GL_TRIANGLES,6,vertex_buffer_data,red,green,blue,GL_FILL);
}
VAO * createLine(float x1,float y1,float x2,float y2,float red,float green,float blue)
{
	GLfloat * vertex_buffer_data = new GLfloat[3*2];
	vertex_buffer_data[0]=x1;
	vertex_buffer_data[1]=y1;
	vertex_buffer_data[2]=0;
	vertex_buffer_data[3]=x2;
	vertex_buffer_data[4]=y2;
	vertex_buffer_data[5]=0;
	return create3DObject(GL_LINES,2,vertex_buffer_data,red,green,blue,GL_LINE);
}
VAO* createCircle(float rad,float cx,float cy,float red,float green,float blue)
{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data[3*i]=cx+(rad* cos(i*M_PI/180.0f));
		vertex_buffer_data[3*i+1]=cy+(rad* sin(i*M_PI/180.0f));
		vertex_buffer_data[3*i+2]=0;
		
		color_buffer_data[3*i]=red;
		color_buffer_data[3*i+1]=green;
		color_buffer_data[3*i+2]=blue;
	}
	return create3DObject(GL_TRIANGLE_FAN, 360 , vertex_buffer_data , color_buffer_data , GL_FILL);
}
VAO* createOval(float radx,float rady,float cx,float cy,float red,float green,float blue)
{
	GLfloat *vertex_buffer_data=new GLfloat[3*360];
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data[3*i]=cx+(radx*cos(i*M_PI/180.0f));
		vertex_buffer_data[3*i+1]=cy+(rady*sin(i*M_PI/180.0f));
		vertex_buffer_data[3*i+2]=0;
	}
	return create3DObject(GL_TRIANGLE_FAN,360,vertex_buffer_data,red,green,blue,GL_FILL);
}
float camera_rotation_angle = 90;
bool flag_ground=false;
float grass1_x=-5.5,grass1_y=4.9,grass2_x=-5.3,grass2_y=4.9,grass3_x=-3.7,grass3_y=3.1,grass4_x=-3.5,grass4_y=3.1;
float cloud_x=4,cloud_y=5;
float cloud1_x=-5.4,cloud1_y=4;
bool cloud_flag=false;
bool cloud1_flag=true;
//float slope1,slope2;
void draw ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (programID);
  Matrices.projection=glm::ortho(zoom*(-6.0f+pan),zoom*(6.0f+pan),zoom*(-6.0f),zoom*(6.0f),0.1f,500.0f);
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
//  Matrices.view=glm::lookAt(glm::vec3(x,1.0f,z),glm::vec3(x+lx,1.0f,z+lz),glm::vec3(0.0f,1.0f,0.0f));
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;	// MVP = Projection * View * Model
  
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateTriangle=glm::translate(glm::vec3(tri_init_x,tri_init_y,0));
  Matrices.model *= translateTriangle; 
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(triangle);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle=glm::translate(glm::vec3(rec_init_x,rec_init_y,0));
  Matrices.model*=translateRectangle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle);

  Matrices.model=glm::mat4(1.0f);
  MVP =VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(bird);
  
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircle=glm::translate(glm::vec3(circle_init_x,circle_init_y,0));
  Matrices.model*=translateCircle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1,GL_FALSE,&MVP[0][0]);
  draw3DObject(circle);

  grass1_x+=0.008;
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translategrass1=glm::translate(glm::vec3(grass1_x,grass1_y,0));
  Matrices.model*=translategrass1;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(grass1);

  grass2_x+=0.008;
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translategrass2=glm::translate(glm::vec3(grass2_x,grass2_y,0));
  Matrices.model*=translategrass2;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(grass2);

  grass3_x+=0.004;
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translategrass3=glm::translate(glm::vec3(grass3_x,grass3_y,0));
  Matrices.model*=translategrass3;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(grass3);

  grass4_x+=0.004;
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translategrass4=glm::translate(glm::vec3(grass4_x,grass4_y,0));
  Matrices.model*=translategrass4;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(grass4);

  if(cloud_flag==false)
  {
	cloud_x-=0.002;
	if(cloud_x<=-5.4)
		cloud_flag=true;
  }
  else if(cloud_flag==true)
  {
	cloud_x+=0.002;
	if(cloud_x>=5.4)
		cloud_flag=false;
  } 
//  cloud_x-=0.002;
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translatecloud=glm::translate(glm::vec3(cloud_x,cloud_y,0));
  Matrices.model*=translatecloud;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(cloud);

  if(cloud1_flag==true)
  {
	cloud1_x+=0.002;
	if(cloud1_x>=5.5)
		cloud1_flag=false;
  }
 else if(cloud1_flag==false)
 {
	cloud1_x-=0.002;
	if(cloud1_x<=-5.5)
		cloud1_flag=true;
  }
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translateCloud1=glm::translate(glm::vec3(cloud1_x,cloud1_y,0));
  Matrices.model*=translateCloud1;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(cloud1);

  Matrices.model=glm::mat4(1.0f);
  glm::mat4 rotateLine=glm::rotate((float)(rot_angle*M_PI/180.0f),glm::vec3(0,0,1));
  glm::mat4 translateLine=glm::translate(glm::vec3(arrow_init_x,arrow_init_y,0));
  Matrices.model*=(translateLine*rotateLine);
  MVP = VP *Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(arrow);
  
  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translatecircle2=glm::translate(glm::vec3(3,-2,0));
  Matrices.model*=translatecircle2;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(obstacle_circle2);

  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translatecircle1=glm::translate(glm::vec3(2,3,0));
  Matrices.model*=translatecircle1;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(obstacle_circle1);

  Matrices.model=glm::mat4(1.0f);
  glm::mat4 translateground=glm::translate(glm::vec3(0,-5.4,0));
  Matrices.model*=translateground;
  MVP = VP *Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(ground);
  
  if(flag==true)
  {
  	Matrices.model=glm::mat4(1.0f);
	glm::mat4 translateBall=glm::translate(glm::vec3(ball_x,ball_y,0));
  	Matrices.model*=translateBall;
  	MVP=VP*Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  	draw3DObject(ball);
  	sleep(0.001);
  	end_t+=0.001;
	if(ball_y>=-4.5)
	{
  	//	cout << "entered 1st case" << endl;
//		if(ball_x>=5.8 || ball_x<=-5.8)
//		{
//			ball_angle=atan(ball_speed_y/ball_speed_x)*180.0f/M_PI;
//			ball_angle+=90;
//			ball_speed_x=-ball_speed_x;
//		}
		ball_y+=-(m*g*end_t/k)+(m*(ball_speed*sin(ball_angle*M_PI/180.0f)+((m*g)/k))/k)*(1-exp(-k*end_t/m));
  		ball_x+=m*ball_speed*cos(ball_angle*M_PI/180.0f)*(1-exp(-k*end_t/m))/k;
		ball_speed_x=ball_speed*cos(ball_angle*M_PI/180.0f)*exp(-k*end_t/m);	    
		ball_speed_y=-(m*g/k)+(ball_speed*sin(ball_angle*M_PI/180.0f)+(m*g/k))*exp(-k*end_t/m);
		if(ball_x>=5.8 || ball_x<=-5.8)
		{	
//			ball_speed_x=-ball_speed_x;
			ball_angle=atan(ball_speed_y/ball_speed_y)*180.0f/M_PI;
			ball_angle+=90;
			if(ball_speed_x>0)
				ball_x=5.7;
			else
				ball_x=-5.7;
		}
	}
	else
	{
	//	cout << "entered 2nd case" << endl;
		ball_y=-4.6;
		ball_speed_y=0;
		if(flag_ground==false)
		{
	//		cout << "grounded first" << endl;
			speed_x=ball_speed_x;	
			flag_ground=true;
		}
		if(ball_x>=5.8 || ball_x<=-5.8)
		{
	//		cout << "hit boundary" << endl;
			if(ball_speed_x*speed_x>0 || ball_speed_x*speed_x<0)
			{	
//				cout << ball_speed_x << endl;
				ball_speed_x=-ball_speed_x;
				if(ball_speed_x>0)
					ball_x=-5.7;
				else
					ball_x=5.7;
			}
			else
			{
				end_t=0;
				flag=false;
				flag_ground=false;
			}
		}
		else if(ball_x<5.8 && ball_x>-5.8)
		{
	//		cout << "not hit boundary" << endl;
			if(ball_speed_x<0 && speed_x*ball_speed_x>0)
			{
	//			cout << "ball with -ve speed" << endl;
				ball_speed_x=speed_x+(e*g*end_t);
				ball_x+=(speed_x*end_t)+(0.5*e*g*end_t*end_t);
			}
			else if(ball_speed_x>0 && speed_x*ball_speed_x>0)
			{
	//			cout << "ball with +ve speed" << endl;
				ball_speed_x=speed_x-(e*g*end_t);
				ball_x+=(speed_x*end_t)-(0.5*e*g*end_t*end_t);
			}
			else if(ball_speed_x*speed_x<=0)
			{
//				cout << "ball with 0 speed "<< endl;
	//			flag=false;
				end_t=0;
				flag_ground=false;
			}
		}
					
	
	}
	if(ball_x>=5.5 && ball_x<=5.7 && ball_y<=0.4 && ball_y>=-0.4)
	{
		score+=10;
		cout << score << endl;
		flag=false;
		time_flight=0;
		end_t=0;
	}	
  	for(int i=0;i<2;i++)
  	{
		  if(sqrt(((ball_x-objects[i].cx)*(ball_x-objects[i].cx))+((ball_y-objects[i].cy)*(ball_y-objects[i].cy)))<=objects[i].rad+0.2)
	  	{	
			score-=5;
			cout << score << endl;
			if(ball_speed_y>=0 && ball_speed_x>=0)
			{	
				if(atan(ball_speed_y/ball_speed_x)==atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx)))
				{
					ball_angle=180+(atan(ball_speed_y/ball_speed_x)*180.0f/M_PI);
					ball_speed_x=-ball_speed_x;
					ball_speed_y=-ball_speed_y;
				}
				else
				{	
					slope1=atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx));
					slope2=atan(ball_speed_y/ball_speed_x);
					if(slope2>slope1)
						ball_angle=90-((2*slope2-3*slope1)*180.0f/M_PI);
					else
						ball_angle=180-((2*slope1-slope2)*180.0f/M_PI);
				}
	//					end_t=0;
			}
			else if(ball_speed_y<=0 && ball_speed_x>=0)		
			{
				if(atan(ball_speed_y/ball_speed_x)==atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx)))
				{
					ball_angle=180-(atan(ball_speed_y/ball_speed_x)*180.0f/M_PI);
					ball_speed_x=-ball_speed_x;
					ball_speed_y=-ball_speed_y;
				}
				else
				{	
					slope1=atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx));
					slope2=atan(ball_speed_y/ball_speed_x);
					if(slope1>slope2)
						ball_angle=180-((2*slope2-slope1)*180.0f/M_PI);
					else if(slope2>slope1)
						ball_angle=180+((2*slope1-slope2)*180.0f/M_PI);
		//			cout << ball_angle << endl;
	//		  	end_t=0;
			}
			}
			else if(ball_speed_x<=0 && ball_speed_y<=0)
			{	
				if(atan(ball_speed_y/ball_speed_x)==atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx)))
				{
					ball_angle=(atan(ball_speed_y/ball_speed_x)*180.0f/M_PI)-180;
					ball_speed_x=-ball_speed_x;
					ball_speed_y=-ball_speed_y;
				}
				else
				{
//					ball_angle=180-(atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx))*180.0f/M_PI);
	//			end_t=0;
					slope1=atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx));
					slope2=atan(ball_speed_y/ball_speed_x);
					if(slope1>slope2)
						ball_angle=180-((2*slope1-slope2)*180.0f/M_PI);
					else if(slope2>slope1)
						ball_angle=180-((2*slope2-slope1)*180.0f/M_PI);
				}
			}
			else if(ball_speed_x<=0 && ball_speed_y>=0)
			{	
				if(atan(ball_speed_y/ball_speed_x)==atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx)))
				{
					ball_angle=180-(atan(ball_speed_y/ball_speed_x)*180.0f/M_PI);
					ball_speed_x=-ball_speed_x;
					ball_speed_y=-ball_speed_y;
				}
				else	
				{
					slope1=atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx));
					slope2=atan(ball_speed_y/ball_speed_x);
					if(slope1>slope2)
						ball_angle=180-((2*atan((ball_y-objects[i].cy)/(ball_x-objects[i].cx))+atan(ball_speed_y/ball_speed_x))*180.0f/M_PI);
					else if(slope2>slope1)
						ball_angle=180-((2*slope2-slope1)*180.0f/M_PI);
				}
	//			end_t=0;
			}
				break;
	  	}		
  	}
  }
  glutSwapBuffers ();
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    draw (); // drawing same scene
}


void initGLUT (int& argc, char** argv, int width, int height)
{
    glutInit (&argc, argv);

    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Angry Bird Game");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);
 
    glutSpecialFunc (keyboardSpecialDown);
//    glutSpecialUpFunc (keyboardSpecialUp);

    glutMotionFunc(scrollMouse);
    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);
    glutMotionFunc(mouseMove);
    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)   
    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


void initGL (int width, int height)
{
	// Create the models
//	sprintf(s,"SCORE : %d",score);
	triangle=createTriangle (-0.75f,-0.433f,0.75f,-0.433f,0,0.866,0.8039,0.5215,0.247); // Generate the VAO, VBOs, vertices data & copy into the array buffer
//        obstacle_tri=createTriangle(0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0,1,0);
	objects[0].cx=3;
	objects[0].cy=-2;
	objects[0].rad=0.6;
	objects[1].cx=2;
	objects[1].cy=3;
	objects[1].rad=0.5;
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.529f, 0.807f, 0.921f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	rectangle=createRectangle (-1.15f,-0.5f,1.15f,-0.5f,1.15f,0.5f,-1.15f,0.5f,0.54509803921,0.27058823529,0.07450980392);
	obstacle_circle2=createCircle(0.6f,0.0f,0.0f,0,0,1);
	circle=createCircle(0.3f,0.0f,0.0f,0,0,1);
	obstacle_circle1=createCircle(0.5f,0.0f,0.0f,0,0,1);
	arrow=createLine(0.0f,0.0f,1.0f,0.0f,0,0,0);
	ground=createRectangle(-6.0f,-0.6f,6.0f,-0.6f,6.0f,0.6f,-6.0f,0.6f,0,1,0);
	ball=createCircle(0.2f,0.0f,0.0f,0.66274509803,0.66274509803,0.66274509803);
	bird=createRectangle(5.7f,-0.2f,5.9f,-0.2f,5.9f,0.2f,5.7f,0.2f,0.95686274509,0.6431372549,0.37647058823);
	grass1=createLine(0.1f,-0.1f,-0.1f,0.1f,0,0,0);
	grass2=createLine(-0.1f,-0.1f,0.1f,0.1f,0,0,0);
	grass3=createLine(0.1f,-0.1f,-0.1f,0.1f,0,0,0);
	grass4=createLine(-0.1f,-0.1f,0.1f,0.1f,0,0,0);
	cloud=createOval(1.2f,0.5f,0.0f,0.0f,1,1,1);
	cloud1=createOval(1.0f,0.4f,0.0f,0.0f,1,1,1);
	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

	glutMainLoop ();

    return 0;
}
