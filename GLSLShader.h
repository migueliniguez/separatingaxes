/*
 * Copyright (c) 2005-2013 Michael Shafae
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * $Id: GLSLShader.h 3372 2012-01-10 21:30:21Z mshafae $
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#ifdef __APPLE__
/* Apple's weird location of their OpenGL & GLUT implementation */
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "misc.h"


class Shader{
public:
  GLuint _object;
  
  Shader( ){ };

  ~Shader( ){
    glDeleteShader( _object );
    msglError( );
  }

  bool compileShader( const GLchar *src ){
    GLint compiled_ok;
    char *msg;
    GLint length = (GLint)strlen(src);
    glShaderSource( _object, 1, &src, &length );
    glCompileShader( _object );
    msglError( );
    glGetShaderiv( _object, GL_COMPILE_STATUS, &compiled_ok );
    msglError( );
    if( !compiled_ok ){
      fprintf( stderr, "Compilation failed for vertex shader\n" );
      msg = getInfoLog( );
      fprintf( stderr, "Info Log:\n%s\n", msg );
      free( msg );
    }
    return( (bool)compiled_ok );
  }
  
  char* getInfoLog( ){
    GLint info_log_length;
    glGetShaderiv( _object, GL_INFO_LOG_LENGTH, &info_log_length );
    char *info_log = (char*)malloc( info_log_length );
    glGetShaderInfoLog( _object, info_log_length, NULL, info_log);
    return( info_log );
  }
};

class VertexShader : public Shader{

public:
  VertexShader( const char *srcFileName ){
    char *src = file2strings( srcFileName );
    if( (Shader::_object = glCreateShader( GL_VERTEX_SHADER )) == 0 ){
      fprintf( stderr, "Can't generate vertex shader name\n" );
    }
    msglError( );
    compileShader( src );
    msglError( );
    free( src );
  }

  GLuint object( ){
    return Shader::_object;
  }

};


class FragmentShader : public Shader{

public:
  FragmentShader( const char *srcFileName ){
    char *src = file2strings( srcFileName );
    if( (Shader::_object = glCreateShader( GL_FRAGMENT_SHADER )) == 0 ){
      fprintf( stderr, "Can't generate fragment shader name\n" );
      exit(1);
    }
      compileShader( src );
      free( src );
    }

    GLuint object( ){
      return Shader::_object;
    }
};


class GLSLProgram{

public:
  GLuint _object;

  GLSLProgram( ){
    if( (_object = glCreateProgram( )) == 0 ){
      fprintf( stderr, "Can't generate program name\n" );
      exit(1);
    }
  }

  ~GLSLProgram( ){
    detachAll( );
    glDeleteProgram( _object );
  }

  bool attach( FragmentShader &fs ){
    glAttachShader( _object, fs.object( ) );
    return( !msglError( ) );
  }

  bool attach( VertexShader &vs ){
    glAttachShader( _object, vs.object( ) );
    return( !msglError( ) );
  }

  bool detachAll( ){
    bool ret = false;
    GLsizei const maxCount = 32;
    GLsizei count;
    GLuint shaders[32];
    glGetAttachedShaders( _object, maxCount, &count, shaders );
    for(int i = 0; i < count; i++ ){
      if( !(detach( shaders[i] )) ){
        ret = true;
      }
    }
    return( ret );
  }

  bool detach( GLuint shaderName ){
    glDetachShader( _object, shaderName );
    return( !msglError( ) );
  }

  bool detach( FragmentShader &fs ){
    glDetachShader( _object, fs.object( ) );
    return( !msglError( ) );
  }

  bool detach( VertexShader &vs ){
    glDetachShader( _object, vs.object( ) );
    return( !msglError( ) );
  }

  bool link( ){
    GLint linked_ok;
    char *msg;
    bool ret = true;
  
    glLinkProgram( _object );

    glGetProgramiv( _object, GL_LINK_STATUS, &linked_ok );
    if( !linked_ok ){
      ret = false;
      fprintf( stderr, "Linking failed.\n" );
      msg = getInfoLog( );
      fprintf( stderr, "%s\n", msg );
      free(msg );
    }
    return ret;
  }
  
  char* getInfoLog( ){
    GLint info_log_length;
    glGetProgramiv( _object, GL_INFO_LOG_LENGTH, &info_log_length );
    char *info_log = (char*)malloc( info_log_length );
    glGetProgramInfoLog( _object, info_log_length, NULL, info_log);
    return( info_log );
  }
  
  bool activate( ){
    glUseProgram( _object );
    return( !msglError( ) );
  }
  
  bool isHardwareAccelerated( ){
    bool ret = true;
     #if defined(__APPLE__)
     /*
      * This checks to make sure that the shaders are going to
      * be hardware accelerated.
      * If they aren't, a message is printed.
      *
      * Something similar can be done on Unix as well as MS Windows but
      * it is left up to the student to look up the neccessary API calls.
      */
    GLint hardwareAccelerated;

    CGLGetParameter( CGLGetCurrentContext( ), kCGLCPGPUVertexProcessing, &hardwareAccelerated );
    if( !hardwareAccelerated ){
      ret = false;
      fprintf( stderr,
        "Warning: Vertex shader is NOT being hardware-accelerated\n" );
    }
    CGLGetParameter( CGLGetCurrentContext( ), kCGLCPGPUFragmentProcessing, &hardwareAccelerated );
    if( !hardwareAccelerated ){
      ret = false;
      fprintf( stderr,
        "Warning: Fragment shader is NOT being hardware-accelerated\n" );
    }
     #endif
    return ret;
  }
};