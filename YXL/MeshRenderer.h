#pragma once

#include "TriMesh.h"

//opengl
#include <gl\glew.h>
#include <GLFW\glfw3.h>

#ifdef _USE_OWN_GL_CONTEXT_
#ifndef GL_LIB
#ifdef _DEBUG
#define GL_LIB(name) name "d.lib"
#else
#define GL_LIB(name) name ".lib"
#endif
#endif
#pragma comment(lib, GL_LIB("glew32"))
#pragma comment(lib, GL_LIB("glfw3"))
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

#ifdef _YXL_DLL_
#ifndef YXL_EXPORT
#ifdef _YXL_EXPORT_IMPL
#define YXL_EXPORT __declspec(dllexport)
#else
#define YXL_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define YXL_EXPORT
#endif

namespace YXL
{
	enum DRAW_TYPE
	{
		DRAW_TYPE_NO_TEXTURED = 0x1
	};

	struct YXL_EXPORT MeshRenderInfo
	{
		std::shared_ptr<TriMesh> mesh = nullptr;
		bool is_reverse_Z = true;
		cv::Mat mat_proj;
		cv::Mat mat_view;
		cv::Mat mat_model = cv::Mat::eye(4, 4, CV_32FC1);
		cv::Size fbo_size;
		int draw_type = DRAW_TYPE_NO_TEXTURED;
		bool need_cpu_result = false;
		bool use_fbo = false;

		MeshRenderInfo();
		void ResetViewAndProjMat();
	};

	void MeshRendererRenderThread(void* param);

	class YXL_EXPORT MeshRenderer
	{
		friend void MeshRendererRenderThread(void* param);

		struct FBO
		{
			FBO() { }
			FBO(cv::Size size) :size(size) { }
			GLuint fbo = 0u;
			GLuint tex = 0u;
			GLuint depth_buffer = 0u;
			cv::Size size;

			cv::Mat render_rendered_img;
			void Init();
		};
		struct VAO
		{
			GLuint vao = 0u;
			GLuint vbo = 0u;
			GLuint ebo = 0u;
			bool is_reverse_Z = true;

			void Init(std::shared_ptr<TriMesh> mesh);

			void Update(std::shared_ptr<TriMesh> mesh, bool is_reverse_Z);

		private:
			void UpdateTriangles(std::shared_ptr<TriMesh> mesh, bool is_reverse_Z);
			void UpdateVertices(std::shared_ptr<TriMesh> mesh);
		};

		static std::map<std::string, GLuint> s_textures;

	public:
		MeshRenderer();
		~MeshRenderer();

		void SetRenderInfo(MeshRenderInfo& render_info);
		void SetRenderInfo(std::shared_ptr<TriMesh> mesh);
		void SetRenderInfo(bool is_reverse_Z);
		void SetRenderInfo(int draw_type);
		void SetRenderInfo(cv::Mat mat_proj, cv::Mat mat_view, cv::Mat mat_model);
		void SetRenderInfo(cv::Size fbo_size);
		void SetRenderInfoProjMat(cv::Mat mat);
		void SetRenderInfoViewMat(cv::Mat mat);
		void SetRenderInfoModelMat(cv::Mat mat);

		cv::Mat Draw();
		void Render();

	protected:
		virtual void GetLightColor(std::vector<cv::Vec4f>& light_color);
		virtual void GetRenderFlags(std::vector<bool>& is_render);
		virtual GLuint GetProgram(bool with_texture);

	private:
		void Init();
		void InitProgram();

		GLuint GetTexture(CStr& path);
		FBO& GetFbo(cv::Size size);
		VAO& GetVao(std::shared_ptr<TriMesh> mesh);


		static GLuint CompileShaderFromFile(int type, CStr& path);
		static GLuint CompileShaderFromString(int type, CStr& source);

		long long Size2LongLong(cv::Size size) const;

		//thread based
	private:
#ifdef _USE_OWN_GL_CONTEXT_
		void RenderThread();
#endif
		virtual void RealRender();
		VAO& SetRenderMesh(std::shared_ptr<TriMesh> mesh);
		void SetGLStates();
		void RenderNormally(FBO& fbo);
		void GetViewNormalMat(cv::Mat& view_mat, cv::Mat& normal_mat);

		void RenderMesh(GLuint prog, cv::Mat normal_mat, cv::Mat view_mat, std::shared_ptr<TriMesh> mesh, const std::vector<bool>& is_render, std::vector<cv::Vec4f>& light_color);
		void RenderGroup(GLuint prog, TriSharedInfo::Mtl& mtl, cv::Mat normal_mat, cv::Mat proj_mat, cv::Mat view_mat, cv::Vec4f light_color, int cnt, int offset);

		void GetRenderResult(FBO& fbo);

	private:
		std::map<long long, FBO> _fbo_list;
		std::map<std::string, VAO> _vao_list;

		GLuint _prog_gray = 0u;
		GLuint _prog_textured = 0u;

		const float Z_NEAR = 0.1f;
		const float Z_FAR = 30000.f;

		//for render thread
	private:
#ifdef _USE_OWN_GL_CONTEXT_
		bool _is_render_thread_to_exit = false;
		CRITICAL_SECTION _cs;

		HANDLE _semaphore_start_render = NULL;
		HANDLE _semaphore_render_done = NULL;
#endif

		MeshRenderInfo _render_info;
		std::map<DWORD, MeshRenderInfo> _pending_render_info;
	};
}

