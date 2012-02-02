/*
 * glsl_shader.cpp
 *
 *  Created on: Nov 27, 2011
 *      Author: hordurj
 */

#include <pcl/simulation/glsl_shader.h>
#include <iostream>
#include <fstream>

namespace gllib
{

char* read_text_file(const char* filename) {
  using namespace std;
  char* buf = NULL;
  ifstream file;
  file.open(filename, ios::in|ios::binary|ios::ate);
  if (file.is_open()) {
    ifstream::pos_type size;
    size = file.tellg();
    buf = new char[size + static_cast<ifstream::pos_type>(1)];
    file.seekg(0, ios::beg);
    file.read(buf, size);
    file.close();
    buf[size] = 0;
  }
  return buf;
}

Program::Program()
{
  program_id_ = glCreateProgram();
}

Program::~Program()
{
}

int Program::get_uniform_location(const std::string& name)
{
  return glGetUniformLocation(program_id_, name.c_str());
}

void Program::set_uniform(const std::string& name, const Eigen::Vector2f& v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform2f(loc, v(0), v(1));
}

void Program::set_uniform(const std::string& name, const Eigen::Vector3f& v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform3f(loc, v(0), v(1), v(2));
}

void Program::set_uniform(const std::string& name, const Eigen::Vector4f& v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform4f(loc, v(0), v(1), v(2), v(4));
}

void Program::set_uniform(const std::string& name, const Eigen::Matrix3f& v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniformMatrix3fv(loc, 1, false, v.data());
}

void Program::set_uniform(const std::string& name, const Eigen::Matrix4f& v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniformMatrix4fv(loc, 1, false, v.data());
}

void Program::set_uniform(const std::string& name, float v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform1f(loc, v);
}

void Program::set_uniform(const std::string& name, int v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform1i(loc, v);
}

void Program::set_uniform(const std::string& name, bool v)
{
  GLuint loc = get_uniform_location(name.c_str());
  glUniform1i(loc, (v?1:0));
}

bool Program::add_shader_text(const std::string& text, ShaderType shader_type)
{
  GLuint id;
  id = glCreateShader(shader_type);
  const char* source_list = text.c_str();

  glShaderSource(id, 1, &source_list, NULL);

  glCompileShader(id);
  print_shader_info_log(id);

  if (get_gl_error() != GL_NO_ERROR) return false;

  glAttachShader(program_id_, id);
}

bool Program::add_shader_file(const std::string& filename, ShaderType shader_type)
{
  char* text = read_text_file(filename.c_str());
  std::string source(text);
  if (text == NULL) return false;
  bool rval = add_shader_text(text, shader_type);
  delete [] text;
  return rval;
}


bool Program::link()
{
  glLinkProgram(program_id_);
  print_program_info_log(program_id_);

  if (get_gl_error() != GL_NO_ERROR) return false;
  return true;
}

void Program::use()
{
  glUseProgram(program_id_);
}

GLenum get_gl_error() {
  GLenum last_error = GL_NO_ERROR;
  GLenum error = glGetError();
  while (error != GL_NO_ERROR) {
    last_error = error;
    std::cout << "Error: OpenGL: " << gluErrorString(error) << std::endl;
    error = glGetError();
  }
  return last_error;
}

void print_shader_info_log(GLuint shader)
{
  GLsizei max_length;
  GLsizei length;
  GLchar* info_log;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  max_length = length;
  info_log = new GLchar[length+1];

  glGetShaderInfoLog(shader, max_length, &length, info_log);

  info_log[max_length] = 0;

  std::cout << "Shader info log: " << std::endl << info_log << std::endl;

  delete [] info_log;
}

void print_program_info_log(GLuint program)
{
  GLsizei max_length;
  GLsizei length;
  GLchar* info_log;

  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
  max_length = length;
  info_log = new GLchar[length+1];

  glGetProgramInfoLog(program, max_length, &length, info_log);

  info_log[max_length] = 0;

  std::cout << "Program info log: " << std::endl << info_log << std::endl;

  delete [] info_log;
}


}
