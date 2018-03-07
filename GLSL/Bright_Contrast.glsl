//author: yixuan, lu

//Photoshop operation: bright and contrast

//bright range: [-150, 150]/255
//contrast range: [-50, 100]/100
//b_c_t=(bright, contrast , 0.4588)

vec4 BrightContrast(vec4 C, vec3 b_c_t)
{
	if(b_c_t.g>=1.0)
		vec4((C.r>b_c_t.b?1.0:0.0),(C.g>b_c_t.b?1.0:0.0),(C.b>b_c_t.b?1.0:0.0),1.0);
	float c=b_c_t.y<=0.0?b_c_t.y:(1.0/(1.0-b_c_t.y)-1.0);
	vec3 new_C=b_c_t.y>0.0?C.rgb+vec3(b_c_t.r):C.rgb;
	new_C=new_C+(new_C-b_c_t.b)*c;
	return b_c_t.y>0.0?vec4(clamp(new_C, vec3(0.0), vec3(1.0)), 1.0):vec4(clamp(new_C+vec3(b_c_t.r), vec3(0.0), vec3(1.0)),1.0);
}