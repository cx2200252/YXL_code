/////////////////////////////////////////////
//
// Mesh Simplification Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
//
//https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//
// 5/2016: Chris Rorden created minimal version for OSX/Linux/Windows compile
#ifndef _MESH_SIMPLIFY_H_
#define _MESH_SIMPLIFY_H_

#include <memory>
#include <vector>

namespace MeshSimplify
{
	struct Float3
	{
		float x, y, z;
	};

	void init(float* v, const int cnt_v, int* tris, int cnt_tri);
	//target_tri_count, target_tri_count: <=0 to ignore, cannot both be ignored
	void process(const int target_tri_count, const int target_vert_count, std::vector<Float3>& fix_points, const int max_iter=100, double agressiveness = 7, bool verbose = false);
	int get_result_vert_num();
	int get_result_tri_num();
	void get_result_data(float* v, int* tris);
};

#endif