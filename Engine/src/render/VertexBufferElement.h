#pragma once

//#include "GL/glew.h"

struct VertexBufferElement {
	unsigned int type;
	unsigned int count;
	bool normalized;

	//static unsigned int GetSizeOfType(unsigned int type)
	//{
	//	switch (type) {
	//	case GL_FLOAT:			return sizeof(GLfloat);
	//	case GL_UNSIGNED_INT:	return sizeof(GLuint);
	//	case GL_UNSIGNED_BYTE:	return sizeof(GLubyte);
	//	}

	//	return 0;
	//}
};