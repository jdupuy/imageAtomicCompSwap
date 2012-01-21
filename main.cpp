////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
// \brief    AMD Catalyst 11.12 bug.
//
////////////////////////////////////////////////////////////////////////////////

// GL libraries
#include "glew.hpp"
#include "GL/freeglut.h"

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

// constants
const GLint ELEM_CNT = 8;

// OpenGL objects
GLuint vertexArray = 0;
GLuint program     = 0;
GLuint buffer      = 0;
GLuint texture     = 0;

// program code
const GLchar* vertexSrc[]={
"#version 420 core\n",

"layout(r32i) coherent uniform iimageBuffer imgData;\n",

"void main() {\n",
	"imageAtomicCompSwap(imgData, gl_VertexID, -1, gl_VertexID);",
"}\n"
};

////////////////////////////////////////////////////////////////////////////////
// Functions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
////////////////////////////////////////////////////////////////////////////////
// Convert GL error code to string
GLvoid gl_debug_message_callback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar* message,
                                 GLvoid* userParam)
{
	std::cerr << "[DEBUG_OUTPUT] "
	          << message
	          << std::endl;
}
#else
const GLchar* gl_error_to_string(GLenum error)
{
	switch(error)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "unknown code";
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init()
{

#ifndef _WIN32
	// Configure debug output
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(
			reinterpret_cast<GLDEBUGPROCARB>(&gl_debug_message_callback),
			NULL );
#endif

	// gen names
	glGenBuffers(1, &buffer);
	glGenTextures(1, &texture);
	glGenVertexArrays(1, &vertexArray);
	program = glCreateProgram();

	// build buffer
	std::vector<GLint> bufferData(ELEM_CNT,-1);
	glBindBuffer(GL_TEXTURE_BUFFER, buffer);
		glBufferData(GL_TEXTURE_BUFFER,
		             sizeof(GLint)*bufferData.size(),
		             &bufferData[0],
		             GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	// build texture
	glBindTexture(GL_TEXTURE_BUFFER, texture);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, buffer);

	// bind as image
	glBindImageTexture(0,
	                   texture,
	                   0,
	                   GL_FALSE,
	                   0,
	                   GL_READ_WRITE,
	                   GL_R32I);

	// build vao
	glBindVertexArray(vertexArray);
		// empty
	glBindVertexArray(0);

	// build program
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 5, vertexSrc, NULL);
	glCompileShader(vertex);
	glAttachShader(program, vertex);
	glDeleteShader(vertex);
	glLinkProgram(program);

	// set uniforms
	glProgramUniform1i(program,
	                   glGetUniformLocation(program, "imgData"),
	                   0);

	// draw data
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_POINTS, 0, ELEM_CNT);

	// map buffer and check data validity
	glBindVertexArray(0);
	glBindBuffer(GL_TEXTURE_BUFFER, buffer);
	GLint *dataPtr = (GLint*) glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_ONLY);
	std::cout << "buffer content : ";
	for(GLint i = 0; i<ELEM_CNT; ++i)
		std::cout << dataPtr[i] << ' ';
	std::cout << std::endl;
	glUnmapBuffer(GL_TEXTURE_BUFFER);

#ifdef _WIN32
	GLenum error = glGetError();
	if(error!=GL_NO_ERROR)
		std::cerr << "caught "
		          << gl_error_to_string(error)
		          << '\n';
#endif

}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean()
{
	// delete objects
	glDeleteBuffers(1, &buffer);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteProgram(program);
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update()
{
	// clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef _WIN32
	GLenum error = glGetError();
	if(error!=GL_NO_ERROR)
		std::cerr << "caught "
		          << gl_error_to_string(error)
		          << '\n';
#endif

	// redraw
	glutSwapBuffers();
	glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
// on resize cb
void on_resize(GLint w, GLint h)
{

}


////////////////////////////////////////////////////////////////////////////////
// on key down cb
void on_key_down(GLubyte key, GLint x, GLint y)
{
	if (key==27) // escape
		glutLeaveMainLoop();
}


////////////////////////////////////////////////////////////////////////////////
// on mouse button cb
void on_mouse_button(GLint button, GLint state, GLint x, GLint y)
{

}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y)
{

}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y)
{

}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	const GLuint CONTEXT_MAJOR = 4;
	const GLuint CONTEXT_MINOR = 2;

	// init glut
	glutInit(&argc, argv);
	glutInitContextVersion(CONTEXT_MAJOR ,CONTEXT_MINOR);

	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// build window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 400);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("unpack");

	// init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		std::stringstream ss;
		ss << err;
		std::cerr << "glewInit() gave error " << ss.str() << std::endl;
		return 1;
	}

	// glewInit generates an INVALID_ENUM error for some reason...
	glGetError();


	// set callbacks
	glutCloseFunc(&on_clean);
	glutReshapeFunc(&on_resize);
	glutDisplayFunc(&on_update);
	glutKeyboardFunc(&on_key_down);
	glutMouseFunc(&on_mouse_button);
	glutPassiveMotionFunc(&on_mouse_motion);
	glutMotionFunc(&on_mouse_motion);
	glutMouseWheelFunc(&on_mouse_wheel);

	// run
	on_init();
	glutMainLoop();

	return 0;
}

