#ifndef _TRI_MESH_H_
#define _TRI_MESH_H_

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

#include "YXLHelper.h"

namespace YXL
{
	class MeshRenderer;
	class TriMesh;

	struct YXL_EXPORT TriSharedInfo
	{
		friend class TriMesh;
		friend class MeshRenderer;
		friend class FaceRigInfoMgr;

	public:
		struct Group
		{
			std::string name = "unnamed";
			std::string mtl_name = "";
			int face_beg = 0;
			int face_end = 0;

			int v_beg = 0;
			int v_end = 0;

			int vt_beg = 0;
			int vt_end = 0;
		};
		struct Mtl
		{
			std::string name;
			std::string map_Ka, map_Kd;
			/*int illum;
			cv::Vec3f Ka, Kd, Ks, Ke;
			float Ns, Ni, d;
			cv::Vec3f Tf;*/
		};

		//org data
	private:
		cv::Mat _org_uvs;
		cv::Mat _org_faces;
		cv::Mat _org_face_uvs;
		std::map<std::string, Group> _org_groups;
		std::map<std::string, Mtl> _mtls;

		//after remove unused points
	private:
		std::map<int, cv::Point3f> _unused_points;
		std::vector<int> _idx_map;

		cv::Mat _tris, _tri_uvs;
		std::map<std::string, Group> _groups;

		//convert to render type (many-to-many vertex-uv mapping)
	private:
		std::map<int, std::vector<int> > _vertex_map_to_render_idx;
		std::map<int, std::vector<int> > _uv_map_to_render_idx;
		cv::Mat _render_tris, _render_uvs;

		//target to render
	private:
		//object index of the face, index of _tris/_render_tris
		std::vector<std::vector<int> > _obj_tris;

		//connected objects' faces in unified indices
		cv::Mat _render_tris_order_by_obj;
		std::vector<int> _render_obj_tri_idx_beg;
		std::vector<std::string> _render_obj_mtls;

	private:
		std::string _identifier;

	public:
		TriSharedInfo(std::string id, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
			std::map<std::string, Group>& groups, std::map<std::string, Mtl>& mtls)
			:_identifier(id),
			_org_uvs(uvs),
			_org_faces(faces),
			_org_face_uvs(face_uvs),
			_org_groups(groups),
			_mtls(mtls) {}

		void Optimise(cv::Mat vertices);

		void SaveAsObj(CStr& path, cv::Mat render_vertices);

	private:
		void SaveMaterial(CStr& path);
		void SaveTexture(CStr& dir);

		void FindUnusedPoints(cv::Mat vertices);
		void ComputeIndexMapping(cv::Mat vertices);
		void FacesToTriangles();
		void GenRenderUVsTriangles();
		void GroupUpTrianglesByConnectivity();
		void ReorderRenderTriangles();
	};

	class YXL_EXPORT TriMesh
	{
		friend class MeshRenderer;

	private:
		static CRITICAL_SECTION s_cs_optimise_infos;
		static std::map<std::string, std::shared_ptr<TriSharedInfo> > s_optimise_infos;

	public:
		static void Init();

		void LoadFromObjFile(CStr& path);
		void Load(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs);
		void Load(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
			std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls);
		void SaveAsObj(CStr& path);
		void UpdateNormal(bool is_reverse_normal=false);
		void UpdateBoundingBox();

		inline std::string GetIdentifier()
		{
			return _identifier;
		}

		int GetObjectCount()
		{
			return _info ? _info->_obj_tris.size():0;
		}
		std::vector<int> GetObjectTriangleList(const int idx)
		{
			return 0 <= idx && idx<GetObjectCount() ? _info->_obj_tris[idx] : std::vector<int>();
		}
		cv::Mat GetVertices()
		{
			return _render_vertices;
		}
		cv::Mat GetTriangles()
		{
			return _info ? _info->_render_tris_order_by_obj : cv::Mat();
		}
		std::shared_ptr<TriMesh> GetSubObject(int idx);


		static std::shared_ptr<TriSharedInfo> GetSharedInfo(const std::string& id);
		std::shared_ptr<TriSharedInfo> GetSharedInfo()
		{
			return GetSharedInfo(_identifier);
		}
		

		cv::Vec3f bbox[2];

		TriMesh() {}
		TriMesh(const TriMesh& mesh)
			:_identifier(mesh._identifier),
			_render_vertices(mesh._render_vertices.clone()),
			_render_normals(mesh._render_normals.clone()),
			_info(mesh._info)
		{
		}
		TriMesh& operator = (const TriMesh& mesh)
		{
			_identifier = mesh._identifier;
			_render_vertices = mesh._render_vertices.clone();
			_render_normals = mesh._render_normals.clone();
			_info = mesh._info;
			return *this;
		}

		

	private:
		void LoadFromObjFile(CStr & path, cv::Mat& vertices, cv::Mat& uvs, cv::Mat& faces, cv::Mat& face_uvs, std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls);

		void LoadMaterial(CStr& path, CStr& obj_dir, std::map<std::string, TriSharedInfo::Mtl>& mtls);
		void GenIdentifier(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
			std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls);
		void OptimiseMesh(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
			std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls);

	private:
		std::string _identifier;
		cv::Mat _render_vertices, _render_normals;

		std::shared_ptr<TriSharedInfo> _info = nullptr;
	};
}

#endif