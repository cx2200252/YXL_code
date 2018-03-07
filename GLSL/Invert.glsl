//author: yixuan, lu

//Photoshop operation: invert

vec4 Invert(vec4 C)
{
	return vec4(1.0-C.xyz, C.w);
}