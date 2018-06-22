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
	void process(int target_tri_count, std::vector<Float3>& fix_points, double agressiveness = 7, bool verbose = false);
	int get_result_vert_num();
	int get_result_tri_num();
	void get_result_data(float* v, int* tris);
};

#endif