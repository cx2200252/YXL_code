#ifdef _YXL_DLL_
#define _YXL_EXPORT_IMPL_
#endif

#include "TriMesh.h"


std::string GetFilePath(CStr& path, CStr& basic_dir)
{
	if (YXL::File::FileExist(path))
	{
		return path;
	}
	else if (YXL::File::FileExist(basic_dir + YXL::File::GetName(path)))
	{
		return basic_dir + YXL::File::GetName(path);
	}
	else
	{
		YXL::yxlout.Lock();
		YXL::yxlout << YXL_LOG_PREFIX << "warning: cannot find file (" << path << ")" << std::endl;
		YXL::yxlout.Unlock();
		return "";
	}
}

void YXL::TriSharedInfo::Optimise(cv::Mat vertices)
{
	FindUnusedPoints(vertices);
	ComputeIndexMapping(vertices);
	FacesToTriangles();
	GenRenderUVsTriangles();
	GroupUpTrianglesByConnectivity();
	ReorderRenderTriangles();
}

void YXL::TriSharedInfo::SaveAsObj(CStr & path, cv::Mat render_vertices)
{
	std::ofstream fout(path);

	if (false == _mtls.empty())
	{
		SaveMaterial(YXL::File::GetPathNE(path) + ".mtl");
		SaveTexture(YXL::File::GetFolder(path));
		fout << "mtllib " << YXL::File::GetNameNE(path) << ".mtl" << std::endl;
	}

	std::map<int, TriSharedInfo::Group> gg;
	for (auto iter = _org_groups.begin(); iter != _org_groups.end(); ++iter)
	{
		gg[iter->second.face_beg] = iter->second;
	}

	for (auto iter = gg.begin(); iter != gg.end(); ++iter)
	{
		TriSharedInfo::Group& g = iter->second;
		/*std::cout << cv::format("%d~%d-%d~%d-%d~%d", g.v_beg, g.v_end, g.vt_beg, g.vt_end, g.face_beg, g.face_end) << std::endl;;*/
		//vertices
		{
			fout << "g default" << std::endl;
			for (int i(g.v_beg); i != g.v_end; ++i)
			{
				int idx = _idx_map[i];
				cv::Point3f vertex;
				if (-1 == idx)
				{
					vertex = _unused_points[i];
				}
				else
				{
					auto tmp = _vertex_map_to_render_idx[idx][0];
					vertex = render_vertices.at<cv::Point3f>(tmp);
				}
				fout << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
			}
			fout << std::endl;
		}
		//uvs
		{
			for (int i(g.vt_beg); i != g.vt_end; ++i)
			{
				cv::Point2f uv = _org_uvs.at<cv::Point2f>(i);
				fout << "vt " << uv.x << " " << uv.y << "\n";
			}
			fout << "\n";
		}

		fout << "g ";
		if (iter->second.name != "unnamed")
		{
			fout << g.name;
		}
		fout << "\n";
		if (iter->second.mtl_name != "")
		{
			fout << "usemtl " << g.mtl_name << "\n";
		}
		fout << "s 1" << "\n";
		for (int i(iter->second.face_beg); i != iter->second.face_end; ++i)
		{
			cv::Vec4i face = _org_faces.at<cv::Vec4i>(i);
			cv::Vec4i face_uv = _org_face_uvs.at<cv::Vec4i>(i);
			face += cv::Vec4i::all(1);
			face_uv += cv::Vec4i::all(1);
			fout << "f ";
			for (int i(0); i != 4; ++i)
			{
				if (face[i] <= 0)
				{
					break;
				}
				fout << face[i] << "/" << face_uv[i] << " ";
			}
			fout << "\n";
		}
		fout << "\n";
	}

	fout.close();
}

void YXL::TriSharedInfo::SaveMaterial(CStr & path)
{
	std::ofstream fout(path);
	for (auto iter = _mtls.begin(); iter != _mtls.end(); ++iter)
	{
		fout << "newmtl " << iter->second.name << std::endl;
		if (true == YXL::File::FileExist(iter->second.map_Ka))
		{
			fout << "\tmap_Ka " << YXL::File::GetName(iter->second.map_Ka) << std::endl;
		}
		if (true == YXL::File::FileExist(iter->second.map_Kd))
		{
			fout << "\tmap_Kd " << YXL::File::GetName(iter->second.map_Kd) << std::endl;
		}
	}

	fout.close();
}

void YXL::TriSharedInfo::SaveTexture(CStr & dir)
{
	for (auto iter = _mtls.begin(); iter != _mtls.end(); ++iter)
	{
		std::string src_path[] = { iter->second.map_Ka, iter->second.map_Kd };
		for (int i(0); i != sizeof(src_path) / sizeof(src_path[0]); ++i)
		{
			std::string dest_path = dir + YXL::File::GetName(src_path[i]);
			if (true == YXL::File::FileExist(src_path[i]) && false == YXL::File::FileExist(dest_path))
			{
				YXL::File::Copy(src_path[i], dest_path);
			}
		}
	}
}

void YXL::TriSharedInfo::FindUnusedPoints(cv::Mat vertices)
{
	YXL::yxlout.Lock();
	YXL::yxlout << YXL_LOG_PREFIX << "finding unused points..." << std::endl;
	YXL::yxlout.Unlock();
	std::vector<int> cnt(vertices.rows);
	for (int i(0); i != _org_faces.rows; ++i)
	{
		cv::Vec4i face = _org_faces.at<cv::Vec4i>(i);
		++cnt[face[0]];
		++cnt[face[1]];
		++cnt[face[2]];
		if (face[3] >= 0)
			++cnt[face[3]];
	}
	_unused_points.clear();
	int idx(0);
	for (int i(0); i != vertices.rows; ++i)
	{
		if (0 == cnt[i])
			_unused_points[i] = vertices.at<cv::Vec3f>(i);
		else
			++idx;
	}
}

void YXL::TriSharedInfo::ComputeIndexMapping(cv::Mat vertices)
{
	YXL::yxlout.Lock();
	YXL::yxlout << YXL_LOG_PREFIX << "computing index mapping..." << std::endl;
	YXL::yxlout.Unlock();
	_idx_map.resize(vertices.rows);
	int cur_idx(0);
	for (int i(0); i != vertices.rows; ++i)
	{
		if (_unused_points.end() != _unused_points.find(i))
		{
			_idx_map[i] = -1;
			continue;
		}
		_idx_map[i] = cur_idx++;
	}
}

void YXL::TriSharedInfo::FacesToTriangles()
{
	//triangles
	_tri_uvs = cv::Mat(0, 1, CV_32SC3);
	_tri_uvs.reserve(_org_face_uvs.rows);
	_tris = cv::Mat(0, 1, CV_32SC3);
	_tris.reserve(_org_faces.rows);
	for (auto iter = _org_groups.begin(); iter != _org_groups.end(); ++iter)
	{
		TriSharedInfo::Group g = iter->second;
		g.face_beg = _tris.rows;
		for (int i(iter->second.face_beg); i != iter->second.face_end; ++i)
		{
			cv::Vec4i face = _org_faces.at<cv::Vec4i>(i);
			cv::Vec4i face_uv = _org_face_uvs.at<cv::Vec4i>(i);
			cv::Vec3i tri;
			cv::Vec3i tri_uv;
			for (int i(0); i != 3; ++i)
			{
				tri[i] = _idx_map[face[i]];
				tri_uv[i] = face_uv[i];
				if (tri[i] < 0 || tri_uv[i] < 0)
				{
					YXL::yxlout.Lock();
					YXL::yxlout << YXL_LOG_PREFIX << "error: vertex index invalid" << cv::format("tri:%d_tri_uv:%d", tri[i], tri_uv[i]) << std::endl;
					YXL::yxlout.Unlock();
					exit(0);
				}
			}
			_tris.push_back(tri);
			_tri_uvs.push_back(tri_uv);
			if (face[3] >= 0)
			{
				tri[1] = tri[2];
				tri[2] = _idx_map[face[3]];
				tri_uv[1] = tri_uv[2];
				tri_uv[2] = face_uv[3];

				_tris.push_back(tri);
				_tri_uvs.push_back(tri_uv);
			}
		}
		g.face_end = _tris.rows;
		_groups[g.name] = g;
	}
}

void YXL::TriSharedInfo::GenRenderUVsTriangles()
{
	std::map<long long, int> mapping;

	_render_tris = cv::Mat(_tris.size(), _tris.type());
	for (int i(0); i != _tris.rows; ++i)
	{
		cv::Vec3i tri = _tris.at<cv::Vec3i>(i);
		cv::Vec3i tri_uv = _tri_uvs.at<cv::Vec3i>(i);
		cv::Vec3i& new_tri = _render_tris.at<cv::Vec3i>(i);
		for (int j(0); j != 3; ++j)
		{
			auto id = YXL::MakeLongLong(tri[j], tri_uv[j]);
			if (mapping.find(id) == mapping.end())
			{
				mapping[id] = mapping.size();
				_render_uvs.push_back(_org_uvs.at<cv::Vec2f>(tri_uv[j]));
			}
			new_tri[j] = mapping[id];
		}
	}

	for (auto iter = mapping.begin(); iter != mapping.end(); ++iter)
	{
		auto id = YXL::SplitLongLong(iter->first);
		_vertex_map_to_render_idx[id.first].push_back(iter->second);
		_uv_map_to_render_idx[id.second].push_back(iter->second);
	}
}

void YXL::TriSharedInfo::TriSharedInfo::GroupUpTrianglesByConnectivity()
{
	int remain_vertex_cnt(0);
	for (auto iter = _idx_map.begin(); iter != _idx_map.end(); ++iter)
	{
		if (*iter>=0)
			++remain_vertex_cnt;
	}

	YXL::UnionFind uf(remain_vertex_cnt);
	for (int i(0); i != _tris.rows; ++i)
	{
		cv::Vec3i tri = _tris.at<cv::Vec3i>(i);
		uf.Union(tri[0], tri[1]);
		uf.Union(tri[0], tri[2]);
		uf.Union(tri[1], tri[2]);
	}
	uf.Update();

	const int obj_cnt = uf.GroupCount();

	_obj_tris.resize(obj_cnt);
	_render_obj_mtls.resize(obj_cnt);
	for (auto iter = _groups.begin(); iter != _groups.end(); ++iter)
	{
		for (int i(iter->second.face_beg); i != iter->second.face_end; ++i)
		{
			cv::Vec3i tri = _tris.at<cv::Vec3i>(i);
			auto group_id = uf.GroupID(tri[0]);
			_obj_tris[group_id].push_back(i);
			_render_obj_mtls[group_id] = iter->second.mtl_name;
		}
	}
}

void YXL::TriSharedInfo::ReorderRenderTriangles()
{
	_render_tris_order_by_obj = cv::Mat(0, 1, CV_32SC3);
	_render_tris_order_by_obj.reserve(_render_tris.rows);

	const int obj_cnt = _obj_tris.size();

	_render_obj_tri_idx_beg.resize(obj_cnt);
	for (int i(0); i != obj_cnt; ++i)
	{
		_render_obj_tri_idx_beg[i] = _render_tris_order_by_obj.rows;
		for (auto iter = _obj_tris[i].begin(); iter != _obj_tris[i].end(); ++iter)
		{
			_render_tris_order_by_obj.push_back(_render_tris.at<cv::Vec3i>(*iter));
		}
	}
}

CRITICAL_SECTION YXL::TriMesh::s_cs_optimise_infos;
std::map<std::string, std::shared_ptr<YXL::TriSharedInfo> > YXL::TriMesh::s_optimise_infos;

void YXL::TriMesh::Init()
{
	InitializeCriticalSection(&s_cs_optimise_infos);
}

void YXL::TriMesh::LoadFromObjFile(CStr & path)
{
	cv::Mat vertices, uvs, faces, face_uvs;
	std::map<std::string, TriSharedInfo::Group> groups;
	std::map<std::string, TriSharedInfo::Mtl> mtls;

	LoadFromObjFile(path, vertices, uvs, faces, face_uvs, groups, mtls);
	Load(vertices, uvs, faces, face_uvs, groups, mtls);
}

void YXL::TriMesh::Load(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs)
{
	
	std::map<std::string, TriSharedInfo::TriSharedInfo::Group> groups;
	auto& g = groups["default"];
	g.face_beg = 0;
	g.face_end = faces.rows;
	g.mtl_name = "default";
	g.name = "default";
	g.vt_beg = 0;
	g.vt_end = 0;
	g.v_beg = 0;
	g.v_end = vertices.rows;

	std::map<std::string, TriSharedInfo::TriSharedInfo::Mtl> mtls;
	Load(vertices, uvs, faces, face_uvs, groups, mtls);
}

void YXL::TriMesh::Load(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs, std::map<std::string, TriSharedInfo::TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::TriSharedInfo::Mtl>& mtls)
{
	if (uvs.empty())
		uvs = cv::Mat(vertices.size(), CV_32FC2, cv::Scalar::all(0));
	if (face_uvs.empty() && vertices.rows == uvs.rows)
	{
		face_uvs = faces.clone();
	}
	CV_Assert(faces.rows == face_uvs.rows);

	GenIdentifier(vertices, uvs, faces, face_uvs, groups, mtls);
	OptimiseMesh(vertices, uvs, faces, face_uvs, groups, mtls);

	YXL::yxlout.Lock();
	YXL::yxlout << YXL_LOG_PREFIX << "groups: ";
	for (auto iter = _info->_org_groups.begin(); iter != _info->_org_groups.end(); ++iter)
	{
		YXL::yxlout << cv::format("(%s_%s_%d-%d)", iter->second.name.c_str(), iter->second.mtl_name.c_str(), iter->second.face_beg, iter->second.face_end);
	}
	YXL::yxlout << std::endl;
	YXL::yxlout.Unlock();
}

void YXL::TriMesh::SaveAsObj(CStr & path)
{
	_info->SaveAsObj(path, _render_vertices);
}

void YXL::TriMesh::UpdateNormal(bool is_reverse_normal)
{
	_render_normals = YXL::ComputeNormal(_render_vertices, _info->_render_tris, is_reverse_normal);
}

void YXL::TriMesh::UpdateBoundingBox()
{
	bbox[0] = cv::Vec3f::all(1.0e10);
	bbox[1] = cv::Vec3f::all(-1.0e10);

	for (int i(0); i != _render_vertices.rows; ++i)
	{
		cv::Vec3f& v = _render_vertices.at<cv::Vec3f>(i);
		for (int j(0); j != 3; ++j)
		{
			bbox[0][j] = (std::min)(bbox[0][j], v[j]);
			bbox[1][j] = (std::max)(bbox[1][j], v[j]);
		}
	}
}

std::shared_ptr<YXL::TriMesh> YXL::TriMesh::GetSubObject(int idx)
{
	if (0 > idx || idx >= GetObjectCount())
		return nullptr;
	auto tri_list = GetObjectTriangleList(idx);
	auto tris = _info->_tris;

	std::map<int, int> idx_map;
	cv::Mat tris_new(0, 1, CV_32SC4);
	for (auto tmp : tri_list)
	{
		auto tri = tris.at<cv::Vec3i>(tmp);
		cv::Vec4i tri2;
		for (int i(0); i != 3; ++i)
		{
			if (idx_map.find(tri[i]) == idx_map.end())
				idx_map[tri[i]] = idx_map.size();
			tri2[i] = idx_map[tri[i]];
		}
		tri2[3] = -1;
		tris_new.push_back(tri2);
	}

	cv::Mat vertices_new(idx_map.size(), 1, CV_32FC3);
	for (auto pair : idx_map)
		vertices_new.at<cv::Vec3f>(pair.second) = _render_vertices.at<cv::Vec3f>(_info->_vertex_map_to_render_idx[pair.first][0]);

	std::shared_ptr<YXL::TriMesh> ret(new YXL::TriMesh);
	ret->Load(vertices_new, cv::Mat(), tris_new, cv::Mat());
	return ret;
}

std::shared_ptr<YXL::TriSharedInfo> YXL::TriMesh::GetSharedInfo(const std::string& id)
{
	return s_optimise_infos[id];
}

void YXL::TriMesh::LoadFromObjFile(CStr & path, cv::Mat& vertices, cv::Mat& uvs, cv::Mat& faces, cv::Mat& face_uvs, std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls)
{
	using namespace std;
	using namespace cv;
	if (_info)
	{
		_info = nullptr;
		_identifier = "";
		_render_vertices = cv::Mat();
		_render_normals = cv::Mat();
	}

	ifstream fin(path);
	if (fin.bad())
	{
		return;
	}

	TriSharedInfo::Group g;
	string line;

	int pre_v_end(0), pre_vt_end(0);
	int max_vt_idx(0);
	while (getline(fin, line))
	{
		stringstream ss;
		ss << line;
		string str;
		ss >> str;
		if ("mtllib" == str)
		{
			string mtl_path;
			ss >> mtl_path;
			LoadMaterial(mtl_path, YXL::File::GetFolder(path), mtls);
		}
		else if ("v" == str)
		{
			Point3f pt;
			ss >> pt.x >> pt.y >> pt.z;
			vertices.push_back(pt);
		}
		else if ("vt" == str)
		{
			Point2f uv;
			ss >> uv.x >> uv.y;
			uvs.push_back(uv);
		}
		else if ("g" == str)
		{
			string name;
			ss >> name;
			if ("unnamed" != g.name && g.face_beg<faces.rows)
			{
				g.face_end = faces.rows;
				g.v_end = vertices.rows;
				g.vt_end = uvs.rows;

				if (max_vt_idx + 1 != uvs.rows)
				{
					YXL::yxlout.Lock();
					YXL::yxlout << YXL_LOG_PREFIX << "error: uv index wrong..." << std::endl;
					YXL::yxlout.Unlock();
					CV_Assert(max_vt_idx + 1 == uvs.rows);
				}

				pre_v_end = vertices.rows;
				pre_vt_end = uvs.rows;

				groups[g.name] = g;
			}
			if (groups.find(name) != groups.end())
			{
				YXL::yxlout.Lock();
				YXL::yxlout << YXL_LOG_PREFIX << "warning: duplicate group name(" << name << ")" << std::endl;;
				YXL::yxlout.Unlock();
			}

			{
				g = TriSharedInfo::Group();
				g.name = name;
				g.face_beg = faces.rows;
				g.v_beg = pre_v_end;
				g.vt_beg = pre_vt_end;
			}
		}
		else if ("usemtl" == str)
		{
			g.mtl_name = line.substr(7, str.length() - 7);
		}
		else if ("f" == str)
		{
			char ch;
			Vec4i face;
			Vec4i face_uv;
			int tmp;
			for (int i(0); i != 4; ++i)
			{
				if (ss.tellg() >= line.length())
				{
					break;
				}
				ss >> face[i];
				ss.get(ch);
				if (' ' == ch)
					continue;
				ss >> face_uv[i];
				ss.get(ch);
				if (' ' == ch)
					continue;
				ss >> tmp;
				ss.get(ch);
				if (' ' == ch)
					continue;
			}
			face -= Vec4i::all(1);
			face_uv -= Vec4i::all(1);
			faces.push_back(face);
			face_uvs.push_back(face_uv);
			for (int i(0); i != 4; ++i)
			{
				if (face_uv[i] >= 0)
					max_vt_idx = (std::max)(max_vt_idx, face_uv[i]);
			}
		}
	}
	fin.close();
	g.face_end = faces.rows;
	g.v_end = vertices.rows;
	g.vt_end = uvs.rows;
	groups[g.name] = g;
}

void YXL::TriMesh::LoadMaterial(CStr & path, CStr& obj_dir, std::map<std::string, TriSharedInfo::Mtl>& mtls)
{
	using namespace std;
	using namespace cv;

	string mtl_path = GetFilePath(path, obj_dir);

	ifstream fin(mtl_path);
	if (fin.bad())
	{
		return;
	}

	TriSharedInfo::Mtl mtl;
	string line;
	while (getline(fin, line))
	{
		stringstream ss;
		ss << line;
		string str;
		ss >> str;
		if ("newmtl" == str)
		{
			string name = line.substr(7, str.length() - 7);
			if ("" != mtl.name)
			{
				mtls[mtl.name] = mtl;
			}

			if (mtls.find(name) != mtls.end())
			{
				YXL::yxlout.Lock();
				YXL::yxlout << YXL_LOG_PREFIX << "warning: duplicate material name(" << name << ")";
				YXL::yxlout.Unlock();
				mtl = mtls[name];
			}
			else
			{
				mtl = TriSharedInfo::Mtl();
				mtl.name = name;
			}
		}
		/*else if ("Ns" == str)
		{
		ss >> mtl.Ns;
		}
		else if ("Ni" == str)
		{
		ss >> mtl.Ni;
		}
		else if ("d" == str)
		{
		ss >> mtl.d;
		}
		else if ("illum" == str)
		{
		ss >> mtl.illum;
		}
		else if ("Tf" == str)
		{
		mtl.Tf = GetMaterialVec3f(ss);
		}
		else if ("Ka" == str)
		{
		mtl.Ka = GetMaterialVec3f(ss);
		}
		else if ("Kd" == str)
		{
		mtl.Kd = GetMaterialVec3f(ss);
		}
		else if ("Ks" == str)
		{
		mtl.Ks = GetMaterialVec3f(ss);
		}
		else if ("Ke" == str)
		{
		mtl.Ke = GetMaterialVec3f(ss);
		}*/
		else if ("map_Ka" == str)
		{
			ss >> mtl.map_Ka;
			mtl.map_Ka = GetFilePath(mtl.map_Ka, obj_dir);
		}
		else if ("map_Kd" == str)
		{
			ss >> mtl.map_Kd;
			mtl.map_Kd = GetFilePath(mtl.map_Kd, obj_dir);
		}
	}
	fin.close();

	mtls[mtl.name] = mtl;

	YXL::yxlout.Lock();
	YXL::yxlout << YXL_LOG_PREFIX << "materials: ";
	for (auto iter = mtls.begin(); iter != mtls.end(); ++iter)
	{
		YXL::yxlout << cv::format("(%s_%s) ", iter->second.map_Ka.c_str(), iter->second.map_Kd.c_str());
	}
	YXL::yxlout << std::endl;
	YXL::yxlout.Unlock();
}

void YXL::TriMesh::GenIdentifier(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
	std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls)
{
	std::string id = cv::format("%d_%d", vertices.rows, faces.rows);

	std::map<std::string, int> m;
	for (int i(0); i != faces.rows; ++i)
	{
		cv::Vec4i face = faces.at<cv::Vec4i>(i);
		m[cv::format("%d_%d_%d_%d", face[0], face[1], face[2], face[3])] = 0;
	}
	std::stringstream ss;
	for (auto iter = m.begin(); iter != m.end(); ++iter)
	{
		ss << iter->first;
	}

	/*for (auto iter = groups.begin(); iter != groups.end(); ++iter)
	{
	id += cv::format("_%s_%s", iter->second.name.c_str(), iter->second.mtl_name.c_str());
	}
	for (auto iter = mtls.begin(); iter != mtls.end(); ++iter)
	{
	id += cv::format("_%s_%s_%s", iter->second.name.c_str(), CmFile::GetName(iter->second.map_Ka).c_str(), CmFile::GetName(iter->second.map_Kd).c_str());
	}*/

	_identifier = YXL::SHA1Digest(id + ss.str());
	YXL::yxlout.Lock();
	YXL::yxlout << YXL_LOG_PREFIX << "object identifier(" << _identifier << "): " << id << std::endl;
	YXL::yxlout.Unlock();

	/*EnterCriticalSection(&s_cs_optimise_infos);
	YXL::yxlout << id<<"\t"<< _identifier << std::endl;
	LeaveCriticalSection(&s_cs_optimise_infos);*/
}

void YXL::TriMesh::OptimiseMesh(cv::Mat vertices, cv::Mat uvs, cv::Mat faces, cv::Mat face_uvs,
	std::map<std::string, TriSharedInfo::Group>& groups, std::map<std::string, TriSharedInfo::Mtl>& mtls)
{
	bool new_model_type = false;
	EnterCriticalSection(&s_cs_optimise_infos);
	if (s_optimise_infos.find(_identifier) == s_optimise_infos.end())
	{
		s_optimise_infos[_identifier] = std::shared_ptr<TriSharedInfo>(new TriSharedInfo(_identifier, uvs, faces, face_uvs, groups, mtls));
		s_optimise_infos[_identifier]->Optimise(vertices);
		new_model_type = true;
		YXL::yxlout.Lock();
		YXL::yxlout << YXL_LOG_PREFIX << "new model type" << std::endl;
		YXL::yxlout.Unlock();
	}
	LeaveCriticalSection(&s_cs_optimise_infos);

	_info = s_optimise_infos[_identifier];

	//_vertices
	{
		_render_vertices = cv::Mat(_info->_render_uvs.rows, 1, CV_32FC3);
		for (int i(0), vertex_cnt(0); i != vertices.rows; ++i)
		{
			if (_info->_idx_map[i]<0)
				continue;

			auto& vertex = vertices.at<cv::Vec3f>(i);
			auto& _vertex_map_to_render_idx = _info->_vertex_map_to_render_idx[vertex_cnt];
			for (auto iter = _vertex_map_to_render_idx.begin(); iter != _vertex_map_to_render_idx.end(); ++iter)
			{
				_render_vertices.at<cv::Vec3f>(*iter) = vertex;
			}
			++vertex_cnt;
		}
	}
}
