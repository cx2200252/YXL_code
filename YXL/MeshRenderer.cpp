#define _LIB_BS_OPTIMISE_IMPL
#include "MeshRenderer.h"
#include "YXLHelper.h"

#ifdef _USE_OWN_GL_CONTEXT_
#include <process.h>
#endif

namespace YXL
{
#ifdef _USE_OWN_GL_CONTEXT_
	GLFWwindow* g_glfw_window = nullptr;
	void InitOpenGL()
	{
		bool res = glfwInit();
		if (false == res)
		{
			CV_Assert(false && "could not start GLFW3.");
		}
		const std::string title = "no name";

		GLFWwindow* wnd(nullptr);

		glfwWindowHint(GLFW_VISIBLE, 0);
		wnd = glfwCreateWindow(100, 100, title.c_str(), NULL, NULL);
		if (nullptr == wnd)
		{
			glfwTerminate();
			CV_Assert(false && "could not open window with GLFW3.");
		}

		glfwMakeContextCurrent(wnd);
		//glfwHideWindow(wnd);

		glfwSwapInterval(0);

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			CV_Assert(false && "glew initialization failed.");
		}

		g_glfw_window = wnd;
	}
#endif

	const float g_fit_diff_scale = 0.1f;

	MeshRenderInfo::MeshRenderInfo()
	{
		ResetViewAndProjMat();
	}

	void MeshRenderInfo::ResetViewAndProjMat()
	{
		cv::Mat proj = YXL::Mat::Perspective(1280.0/720.0, 0.1f, 3000.0f, 60);

		cv::Mat view = YXL::Mat::LookAt(cv::Vec3f(0, 0, -600.f), cv::Vec3f(0,0,0), cv::Vec3f(0, 1, 0));
		
		//std::cout << proj << "\n" << view << std::endl;

		mat_proj = proj.clone();
		mat_view = view.clone();
		fbo_size = cv::Size(1280, 720);
	}

#ifdef _USE_OWN_GL_CONTEXT_
	void MeshRendererRenderThread(void* param)
	{
		reinterpret_cast<MeshRenderer*>(param)->RenderThread();
	}
#endif

	void MeshRenderer::FBO::Init()
	{
		YXL::yxlout << YXL_LOG_PREFIX << "creating fbo" << std::endl;
		glGenRenderbuffers(1, &depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.width, size.height);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width, size.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glDeleteRenderbuffers(1, &depth_buffer);
			depth_buffer = 0u;
			glDeleteTextures(1, &tex);
			tex = 0u;
			glDeleteFramebuffers(1, &fbo);
			fbo = 0u;
			YXL::yxlout << YXL_LOG_PREFIX << "error::framebuffer..." << std::endl;
		}

		render_rendered_img = cv::Mat(size, CV_8UC4);

		YXL::yxlout << YXL_LOG_PREFIX << "size: " << size << std::endl;
	}

	void MeshRenderer::VAO::Init(std::shared_ptr<TriMesh> mesh)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		const int vertex_cnt = mesh->_render_vertices.rows;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex_cnt * 8, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*vertex_cnt * 3, sizeof(float)*vertex_cnt * 2, mesh->_info->_render_uvs.data);

		Update(mesh, is_reverse_Z);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*vertex_cnt * 3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*vertex_cnt * 5));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		YXL::yxlout << YXL_LOG_PREFIX << "#vertex: " << vertex_cnt << std::endl;

	}

	void MeshRenderer::VAO::Update(std::shared_ptr<TriMesh> mesh, bool is_reverse_Z)
	{
		UpdateTriangles(mesh, is_reverse_Z);
		UpdateVertices(mesh);
		this->is_reverse_Z = is_reverse_Z;
	}

	void MeshRenderer::VAO::UpdateTriangles(std::shared_ptr<TriMesh> mesh, bool is_reverse_Z)
	{
		if (ebo && this->is_reverse_Z == is_reverse_Z)
			return;
		if(!ebo)
			glGenBuffers(1, &ebo);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		std::vector<unsigned int> tris;
		tris.reserve(mesh->_info->_render_tris_order_by_obj.rows * 3);

		cv::Vec3i reorder = is_reverse_Z ? cv::Vec3i(2, 1, 0) : cv::Vec3i(0, 1, 2);

		auto& _render_tris = mesh->_info->_render_tris_order_by_obj;
		for (int i(0); i != _render_tris.rows; ++i)
		{
			cv::Vec3i tri = _render_tris.at<cv::Vec3i>(i);
			for (int j(0); j != 3; ++j)
			{
				tris.push_back(tri[reorder[j]]);
			}
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*tris.size(), &tris[0], GL_STATIC_DRAW);

		YXL::yxlout << YXL_LOG_PREFIX << "#triangle: " << tris.size()/3 << std::endl;
	}

	void MeshRenderer::VAO::UpdateVertices(std::shared_ptr<TriMesh> mesh)
	{
		const int vertex_cnt = mesh->_render_vertices.rows;

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*vertex_cnt * 3, mesh->_render_vertices.data);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*vertex_cnt * 5, sizeof(float)*vertex_cnt * 3, mesh->_render_normals.data);
	}
	
	std::map<std::string, GLuint> MeshRenderer::s_textures;

	MeshRenderer::MeshRenderer()
	{
#ifdef _USE_OWN_GL_CONTEXT_
		_semaphore_start_render = CreateSemaphoreA(NULL, 0, 1, "mesh_renderer_start_render");
		_semaphore_render_done = CreateSemaphoreA(NULL, 0, 1, "mesh_renderer_render_done");

		InitializeCriticalSection(&_cs);

		_beginthread(MeshRendererRenderThread, 0, this);
#endif
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetRenderInfo(MeshRenderInfo & render_info)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info = render_info;
	}

	void MeshRenderer::SetRenderInfo(std::shared_ptr<TriMesh> mesh)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.mesh = mesh;
	}

	void MeshRenderer::SetRenderInfo(bool is_reverse_Z)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.is_reverse_Z = is_reverse_Z;
	}

	void MeshRenderer::SetRenderInfo(int draw_type)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.draw_type = draw_type;
	}

	void MeshRenderer::SetRenderInfo(cv::Mat mat_proj, cv::Mat mat_view, cv::Mat mat_model)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.mat_proj = mat_proj;
		info.mat_view = mat_view;
		info.mat_model = mat_model;
	}

	void MeshRenderer::SetRenderInfo(cv::Size fbo_size)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.fbo_size = fbo_size;
	}

	void MeshRenderer::SetRenderInfoProjMat(cv::Mat mat)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.mat_proj = mat;
	}

	void MeshRenderer::SetRenderInfoViewMat(cv::Mat mat)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.mat_view = mat;
	}

	void MeshRenderer::SetRenderInfoModelMat(cv::Mat mat)
	{
		auto& info = _pending_render_info[YXL::GetCurrentThreadID()];
		info.mat_model = mat;
	}

	cv::Mat MeshRenderer::Draw()
	{
		auto thread_id = YXL::GetCurrentThreadID();
		auto info = _pending_render_info[thread_id];

		info.need_cpu_result = true;
		info.use_fbo = true;
#ifdef _USE_OWN_GL_CONTEXT_
		EnterCriticalSection(&_cs);
		_render_info = info;
		ReleaseSemaphore(_semaphore_start_render, 1, 0);
		WaitForSingleObject(_semaphore_render_done, INFINITE);
		cv::Mat res = GetFbo(_render_info.fbo_size).render_rendered_img.clone();
		LeaveCriticalSection(&_cs);
#else
		_render_info = info;
		RealRender();
		cv::Mat res = GetFbo(_render_info.fbo_size).render_rendered_img.clone();
#endif
		return res;
	}

	void MeshRenderer::Render()
	{
		auto thread_id = YXL::GetCurrentThreadID();
		auto info = _pending_render_info[thread_id];

		info.need_cpu_result = false;
		info.use_fbo = false;
#ifdef _USE_OWN_GL_CONTEXT_
		EnterCriticalSection(&_cs);
		_render_info = info;
		ReleaseSemaphore(_semaphore_start_render, 1, 0);
		WaitForSingleObject(_semaphore_render_done, INFINITE);
		LeaveCriticalSection(&_cs);
#else
		_render_info = info;
		RealRender();
#endif
	}

	void MeshRenderer::GetLightColor(std::vector<cv::Vec4f>& light_color)
	{
		auto obj_cnt = _render_info.mesh->GetObjectCount();
		light_color.resize(obj_cnt, cv::Vec4f::all(1.0f));
	}

	void MeshRenderer::GetRenderFlags(std::vector<bool>& is_render)
	{
		auto obj_cnt = _render_info.mesh->GetObjectCount();
		is_render.resize(obj_cnt, true);
	}

	GLuint MeshRenderer::GetProgram(bool with_texture)
	{
		return with_texture ? _prog_textured : _prog_gray;
	}

	void MeshRenderer::Init()
	{
#ifdef _USE_OWN_GL_CONTEXT_
		if (nullptr == g_glfw_window)
		{
			InitOpenGL();
		}
#endif
		InitProgram();
	}

	void MeshRenderer::InitProgram()
	{
		CStr def_vs_gray="#version 330 core\n"
			"layout(location = 0) in vec3 P;\n"
			"layout(location = 1) in vec2 st;\n"
			"layout(location = 2) in vec3 norm;\n"
			"uniform mat4 norm_mat;\n"
			"uniform mat4 view;\n"
			"uniform mat4 proj;\n"
			"out vec2 st_frag;\n"
			"out vec3 normal;\n"
			"out vec3 FragPos;\n"
			"void main()\n"
			"{\n"
			"\tgl_Position = (proj*view*vec4(P, 1.0));\n"
			"\tst_frag = 1.0 - st;\n"
			"\tmat3 norm_mat2=mat3(norm_mat);\n"
			"\tnormal = normalize(norm_mat2*norm);\n"
			"\tFragPos = (view*vec4(P, 1.0)).xyz;\n"
			"}";
		CStr def_fs_gray = "#version 330 core\n"
			"uniform sampler2D tex;\n"
			"uniform vec4 lightColor;\n"
			"uniform int diff_type;\n"

			"in vec2 st_frag;\n"
			"in vec3 normal;\n"
			"in vec3 FragPos;\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"\tvec3 lightPos = vec3(0.0, 0.0, 1000.0);\n"
			"\tvec3 lightDir = normalize(FragPos-lightPos);\n"
			"\tfloat diff = max(dot(normal, lightDir), 0.0);\n"
			"\tvec4 diffuse = vec4(diff*lightColor.rgb, lightColor.a);\n"
			"\tFragColor = diffuse;\n"
			"}\n";
		CStr def_vs_tex = "#version 330 core\n"
			"layout(location = 0) in vec3 P;\n"
			"layout(location = 1) in vec2 st;\n"
			"layout(location = 2) in vec3 norm;\n"
			"uniform mat4 norm_mat;\n"
			"uniform mat4 view;\n"
			"uniform mat4 proj;\n"
			"out vec2 st_frag;\n"
			"out vec3 normal;\n"
			"out vec3 FragPos;\n"
			"void main()\n"
			"{\n"
			"\tgl_Position = (proj*view*vec4(P, 1.0));\n"
			"\tst_frag = 1.0 - st;\n"
			"\tmat3 norm_mat2=mat3(norm_mat);\n"
			"\tnormal = normalize(norm_mat2*norm);\n"
			"\tFragPos = (view*vec4(P, 1.0)).xyz;\n"
			"}";
		CStr def_fs_tex="#version 330 core\n"
			"uniform sampler2D tex;\n"
			"uniform vec4 lightColor;\n"
			"uniform int diff_type;\n"
			"in vec2 st_frag;\n"
			"in vec3 normal;\n"
			"in vec3 FragPos;\n"
		"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"\tvec3 lightPos = vec3(0.0, 0.0, 1000.0);\n"
			"\tvec3 lightDir = normalize(FragPos-lightPos);\n"
			"\tfloat angle = max(dot(normal, lightDir), 0.0);\n"
			"\tvec4 diffuse = vec4(angle*lightColor.rgb, lightColor.a);\n"
			"\tFragColor = diffuse*texture2D(tex, vec2(1.0 - st_frag.x, st_frag.y));\n"
			"}\n";

		CStr def_vss[] = { def_vs_gray , def_vs_tex };
		CStr def_fss[] = { def_fs_gray , def_fs_tex };
		CStr vss[2] = { "vs.glsl", "vs_tex.glsl" };
		CStr fss[2] = { "fs.glsl", "fs_tex.glsl" };
		GLuint* progs[] = { &_prog_gray, &_prog_textured };
		for (int i(0); i != 2; ++i)
		{
			GLuint vs = 0u, fs = 0u;
			if (YXL::File::FileExist(vss[i]))
			{
				vs = CompileShaderFromFile(GL_VERTEX_SHADER, vss[i]);
			}
			else
			{
				vs = CompileShaderFromString(GL_VERTEX_SHADER, def_vss[i]);
				YXL::yxlout << YXL_LOG_PREFIX << "warning: cannot find " << YXL::File::GetWkDir() << "\\"<< vss[i]<<", use default vertex shader." << std::endl;
			}
			if (YXL::File::FileExist(fss[i]))
			{
				fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, fss[i]);
			}
			else
			{
				fs = CompileShaderFromString(GL_FRAGMENT_SHADER, def_fss[i]);
				YXL::yxlout << YXL_LOG_PREFIX << "warning: cannot find " << YXL::File::GetWkDir() << "\\"<< fss[i]<<", use default fragment shader." << std::endl;
			}

			*progs[i] = glCreateProgram();
			glAttachShader(*progs[i], vs);
			glAttachShader(*progs[i], fs);
			glLinkProgram(*progs[i]);

			int  success;
			char infoLog[512];
			glGetProgramiv(*progs[i], GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(*progs[i], 512, NULL, infoLog);
				YXL::yxlout << YXL_LOG_PREFIX << "error: program link failed...\n" << infoLog << std::endl;
			}

			glDeleteShader(vs);
			glDeleteShader(fs);
		}
	}

	GLuint MeshRenderer::GetTexture(CStr & path)
	{
		if (s_textures.find(path) != s_textures.end())
		{
			return s_textures[path];
		}
		if (false == YXL::File::FileExist(path))
		{
			s_textures[path] = 0u;
			YXL::yxlout << YXL_LOG_PREFIX << "error: texture not exixt: " << path << std::endl;
			return 0u;
		}

		GLuint id;
		//load texture
		{
			cv::Mat tex = cv::imread(path, -1);
			if (true == tex.empty())
			{
				s_textures[path] = 0u;
				YXL::yxlout << YXL_LOG_PREFIX << "error: cannot read texture: " << path << std::endl;
				return 0u;
			}
			if (3 == tex.channels())
			{
				cv::cvtColor(tex, tex, CV_BGR2BGRA);
			}
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.cols, tex.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		s_textures[path] = id;
		YXL::yxlout << YXL_LOG_PREFIX << "load texture: " << path << std::endl;

		return id;
	}

	MeshRenderer::FBO& MeshRenderer::GetFbo(cv::Size size)
	{
		if (_fbo_list.find(Size2LongLong(size)) != _fbo_list.end())
		{
			return _fbo_list[Size2LongLong(size)];
		}

		_fbo_list.insert(std::make_pair(Size2LongLong(size), FBO(size)));
		_fbo_list[Size2LongLong(size)].Init();

		return _fbo_list[Size2LongLong(size)];
	}

	MeshRenderer::VAO& MeshRenderer::GetVao(std::shared_ptr<TriMesh> mesh)
	{
		CStr id = mesh->GetIdentifier();
		if (_vao_list.find(id) != _vao_list.end())
		{
			return _vao_list[id];
		}
		_vao_list[id].is_reverse_Z = _render_info.is_reverse_Z;
		_vao_list[id].Init(mesh);

		return _vao_list[id];
	}
	
	GLuint MeshRenderer::CompileShaderFromFile(int type, CStr & path)
	{
		std::ifstream fin(path);
		std::string source, line;
		while (getline(fin, line))
		{
			source += line + "\n";
		}
		fin.close();

		return CompileShaderFromString(type, source);
	}

	GLuint MeshRenderer::CompileShaderFromString(int type, CStr & source)
	{
		GLuint shader = glCreateShader(type);

		int len = source.length();
		auto data = source.c_str();
		glShaderSource(shader, 1, &data, &len);
		glCompileShader(shader);

		int  success;
		char infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			YXL::yxlout << YXL_LOG_PREFIX <<"error: shader compilation failed...\n" << infoLog << std::endl;
		}
		return shader;
	}

	long long MeshRenderer::Size2LongLong(cv::Size size) const
	{
		return YXL::MakeLongLong(size.width, size.height);
	}

#ifdef _USE_OWN_GL_CONTEXT_
	void MeshRenderer::RenderThread()
	{
		while (true)
		{
			WaitForSingleObject(_semaphore_start_render, INFINITE);
			if (_is_render_thread_to_exit)
			{
				break;
			}
			glfwMakeContextCurrent(g_glfw_window);

			RealRender();

			ReleaseSemaphore(_semaphore_render_done, 1, 0);
		}
	}
#endif

	void MeshRenderer::RealRender()
	{
		if (0u == _prog_gray)
		{
			Init();
		}

		if (true == _render_info.use_fbo)
		{
			FBO& fbo = GetFbo(_render_info.fbo_size);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
			glViewport(0, 0, fbo.size.width, fbo.size.height);

			SetGLStates();

			glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
			glClearDepth(Z_FAR);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			RenderNormally(fbo);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			FBO fbo;
			SetGLStates();
			RenderNormally(fbo);
		}
	}

	MeshRenderer::VAO & MeshRenderer::SetRenderMesh(std::shared_ptr<TriMesh> mesh)
	{
		VAO& vao = GetVao(mesh);
		mesh->UpdateNormal(_render_info.is_reverse_Z);
		vao.Update(mesh, _render_info.is_reverse_Z);
		return vao;
	}

	void MeshRenderer::SetGLStates()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		/*glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);*/

		//glClearColor(0.0f, 0.0f, 0.00f, 1.0f);
		/*glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClearDepth(Z_FAR);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
	}

	void MeshRenderer::RenderNormally(FBO& fbo)
	{
		auto draw_type = _render_info.draw_type;
		cv::Mat view_mat, normal_mat;
		GetViewNormalMat(view_mat, normal_mat);

		VAO& vao = SetRenderMesh(_render_info.mesh);
		glBindVertexArray(vao.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);

		auto obj_cnt = _render_info.mesh->GetObjectCount();
		std::vector<bool> is_render;
		GetRenderFlags(is_render);

		std::vector<cv::Vec4f> light_color;
		GetLightColor(light_color);

		RenderMesh(0u, normal_mat, view_mat, _render_info.mesh, is_render, light_color);

		if(_render_info.need_cpu_result && _render_info.use_fbo)
			GetRenderResult(fbo);
	}

	void MeshRenderer::GetViewNormalMat(cv::Mat & view_mat, cv::Mat & normal_mat)
	{
		cv::Mat mat_model = _render_info.mat_model;
		if (mat_model.empty())
		{
			mat_model = cv::Mat::eye(4, 4, CV_32FC1);
		}

		view_mat = mat_model*_render_info.mat_view;
		normal_mat = view_mat.clone();
		normal_mat.at<float>(3, 0) = 0.0f;
		normal_mat.at<float>(3, 1) = 0.0f;
		normal_mat.at<float>(3, 2) = 0.0f;

		normal_mat = normal_mat.inv().t();
		normal_mat = normal_mat.clone();
	}

	void MeshRenderer::RenderMesh(GLuint prog, cv::Mat normal_mat, cv::Mat view_mat, 
		std::shared_ptr<TriMesh> mesh, const std::vector<bool>& is_render, 
		std::vector<cv::Vec4f>& light_color)
	{
		auto draw_type = _render_info.draw_type;
		const int obj_cnt = mesh->GetObjectCount();

		for (int i(0); i != obj_cnt; ++i)
		{
			if (false == is_render[i])
				continue;
			std::string mtl_name = mesh->_info->_render_obj_mtls[i];
			int offset = mesh->_info->_render_obj_tri_idx_beg[i] * 3 * sizeof(int);
			int cnt = ((i + 1 != obj_cnt) ? mesh->_info->_render_obj_tri_idx_beg[i + 1] : mesh->_info->_render_tris_order_by_obj.rows) - mesh->_info->_render_obj_tri_idx_beg[i];
			//YXL::yxlout <<i<< "\toffset: " << offset << "\tcnt: " << cnt << std::endl;

			RenderGroup(prog,
				mesh->_info->_mtls[mtl_name],
				normal_mat,
				_render_info.mat_proj,
				view_mat,
				light_color[i],
				cnt * 3,
				offset);
		}
	}

	void MeshRenderer::RenderGroup(GLuint prog, TriSharedInfo::Mtl & mtl, 
		cv::Mat normal_mat, cv::Mat proj_mat, cv::Mat view_mat,
		cv::Vec4f light_color, int cnt, int offset)
	{
		std::string tex_path = mtl.map_Ka;
		if (false == YXL::File::FileExist(tex_path))
		{
			tex_path = mtl.map_Kd;
		}
		GLuint tex_id = GetTexture(tex_path);

		if (0u == prog)
			prog=GetProgram(0u != tex_id);
		
		glUseProgram(prog);

		glBindTexture(GL_TEXTURE_2D, tex_id);
		{
			int norm_mat_pos = glGetUniformLocation(prog, "norm_mat");
			glUniformMatrix4fv(norm_mat_pos, 1, GL_FALSE, (float*)normal_mat.data);

			int view_pos = glGetUniformLocation(prog, "view");
			glUniformMatrix4fv(view_pos, 1, GL_FALSE, (float*)view_mat.data);

			int proj_pos = glGetUniformLocation(prog, "proj");
			glUniformMatrix4fv(proj_pos, 1, GL_FALSE, (float*)proj_mat.data);

			int tex_pos = glGetUniformLocation(prog, "tex");
			glUniform1i(tex_pos, 0);

			int light_color_pos = glGetUniformLocation(prog, "lightColor");
			glUniform4f(light_color_pos, light_color[0], light_color[1], light_color[2], light_color[3]);

			//std::cout << light_color_pos<<"\t"<<light_color << std::endl;
		}

		glDrawElements(GL_TRIANGLES, cnt, GL_UNSIGNED_INT, (const void*)offset);
	}
	
	void MeshRenderer::GetRenderResult(FBO & fbo)
	{
		glBindTexture(GL_TEXTURE_2D, fbo.tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, fbo.render_rendered_img.data);
	}
}
