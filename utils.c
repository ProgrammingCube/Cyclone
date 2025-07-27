#include "utils.h"

char* readFile( char* file_name )
{
    FILE* file = fopen( file_name, "rb" );
    if ( !file )
    {
        fprintf( stderr, "Init Error: Unable to open file %s\n", file_name );
        return NULL;
    }

    fseek( file, 0, SEEK_END) ;
    long length = ftell( file );
    fseek( file, 0, SEEK_SET );

    char* buffer = ( char* )malloc( length + 1 );
    if ( !buffer )
    {
        fprintf( stderr, "Init Error: Unable to allocate memory for file %s\n", file_name );
        fclose( file );
        return NULL;
    }

    fread( buffer, 1, length, file );
    buffer[ length ] = '\0';
    fclose( file );
    return buffer;
}

char* loadShaderSource( char* file_name )
{
    return readFile( file_name );
}

GLuint compileShader( GLenum shaderType, char* file_name )
{
    GLint success;
    GLuint shader;
    const GLchar* shaderSource = loadShaderSource( file_name );
    if ( shaderSource == NULL )
    {
        fprintf( stderr, "Nonexistant or empty shader file\n" );
        return ( GLuint )0;
    }

    shader = glCreateShader( shaderType );
    glShaderSource( shader, 1, &shaderSource, NULL );
    glCompileShader( shader );
    free( ( void* )shaderSource );

    
    glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
    if ( !success )
    {
        GLchar infoLog[ GL_LOG_LENGTH ];
        glGetShaderInfoLog( shader, GL_LOG_LENGTH, NULL, infoLog );
        fprintf( stderr, "ERROR::SHADER::COMPILATION_FAILED for %s\n%s\n", file_name, infoLog );
        glDeleteShader( shader );
        return ( GLuint )0;
    }

    return shader;
}

GLuint createShaderProgram( char* vertexShader_source, char* fragmentShader_source )
{
    GLint success;
    GLuint vertexShader = compileShader( GL_VERTEX_SHADER, vertexShader_source );
    GLuint fragmentShader = compileShader( GL_FRAGMENT_SHADER, fragmentShader_source );

    GLuint program = glCreateProgram();
    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );
    glLinkProgram( program );
    
    glGetProgramiv( program, GL_LINK_STATUS, &success) ;
    if ( !success )
    {
        GLchar infoLog[ GL_LOG_LENGTH ];
        glGetProgramInfoLog( program, GL_LOG_LENGTH, NULL, infoLog );
        glDeleteShader( vertexShader );
        glDeleteShader( fragmentShader );
        fprintf( stderr, "ERROR:SHADER::PROGRAM::LINKING_FAILED: %s\n", infoLog );
        return ( GLuint )0;
    }

    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );

    return program;
}