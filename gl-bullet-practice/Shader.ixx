module;

#include <gl/glew.h>
#include <gl/GL.h>
#include <string_view>
#include <memory>
#include <vector>
#include <format>
#include <stdexcept>
#include <fstream>

export module Shader;

import TypedRange;

export namespace ownfos
{
	// Interface for classes that provide source code for vertex/fragment shader
	class IShaderSource
	{
	public:
		virtual std::string_view get_shader_code() const = 0;
	};

	// Provide string value as source code
	class ShaderSourceFromString : public IShaderSource
	{
	public:
		ShaderSourceFromString(std::string_view shader_code)
			: shader_code(shader_code)
		{}

		std::string_view get_shader_code() const override
		{
			return shader_code;
		}

	private:
		std::string_view shader_code;
	};

	// Provide content of text file as source code
	class ShaderSourceFromFile : public IShaderSource
	{
	public:
		ShaderSourceFromFile(std::string_view file_name)
		{
			// Open file
			auto file = std::ifstream(file_name.data());
			if (!file.is_open())
			{
				throw std::runtime_error(std::format("[ShaderSourceTextFile] Failed to open file \"{0}\"", file_name));
			}

			// Read content into std::string instance
			auto buffer = std::string(1024, '\0');
			while (file.read(&buffer[0], buffer.size()))
			{
				shader_code.append(buffer, 0, file.gcount());
			}
			shader_code.append(buffer, 0, file.gcount());
		}

		std::string_view get_shader_code() const override
		{
			return shader_code;
		}

	private:
		std::string shader_code;
	};

	// Provide concatenated version of multiple partial source codes.
	class ShaderSourceComposite : public IShaderSource
	{
	public:
		ShaderSourceComposite(const std::vector<IShaderSource*>& shader_source_container)
		{
			for (const IShaderSource* source : shader_source_container)
			{
				concatnated_shader_code.append(source->get_shader_code());
			}
		}

		std::string_view get_shader_code() const override
		{
			return concatnated_shader_code;
		}

	private:
		std::string concatnated_shader_code;
	};

	// Used to check shader preparation failure and provide buffer for error log messages
	template<size_t log_buffer_size = 512>
	struct ShaderInfoLog
	{
		int success;
		std::array<char, log_buffer_size> error_log;
	};

	// Compiles and links vertex shader and fragment shader.
	// Call use() to activate this shader program.
	class ShaderProgram
	{
	public:
		ShaderProgram(const IShaderSource* vertex_shader_source, const IShaderSource* fragment_shader_source)
			: vertex_shader(glCreateShader(GL_VERTEX_SHADER))
			, fragment_shader(glCreateShader(GL_FRAGMENT_SHADER))
			, shader_program(glCreateProgram())
		{
			compile(vertex_shader, vertex_shader_source);
			compile(fragment_shader, fragment_shader_source);
			link();
		}

		void use() const
		{
			glUseProgram(shader_program);
		}

	private:
		void compile(unsigned int& shader, const IShaderSource* source)
		{
			const auto source_code = source->get_shader_code().data();
			glShaderSource(shader, 1, &source_code, nullptr);
			glCompileShader(shader);

			check_compile_error(shader, source);
		}

		void link()
		{
			glAttachShader(shader_program, vertex_shader);
			glAttachShader(shader_program, fragment_shader);
			glLinkProgram(shader_program);

			check_link_error();
		}

		void check_compile_error(unsigned int& shader, const IShaderSource* source)
		{
			auto info = ShaderInfoLog();

			glGetShaderiv(shader, GL_COMPILE_STATUS, &info.success);
			if (!info.success)
			{
				glGetShaderInfoLog(shader, info.error_log.size(), nullptr, info.error_log.data());
				throw std::runtime_error(std::format(
					"[ShaderProgram] Failed to compile shader\n"
					"--------------------------- Source code ---------------------------\n"
					"{0}\n"
					"-------------------------- Error Message --------------------------\n"
					"{1}\n",
					source->get_shader_code(),
					info.error_log.data())
				);
			}
		}

		void check_link_error()
		{
			auto info = ShaderInfoLog();

			glGetProgramiv(shader_program, GL_LINK_STATUS, &info.success);
			if (!info.success) {
				glGetProgramInfoLog(shader_program, info.error_log.size(), nullptr, info.error_log.data());
				throw std::runtime_error(std::format(
					"[ShaderProgram] Failed to link shaders into program\n"
					"-------------------------- Error Message --------------------------\n"
					"{0}",
					info.error_log.data())
				);
			}
		}

		unsigned int vertex_shader;
		unsigned int fragment_shader;
		unsigned int shader_program;
	};
} // namespace ownfos
