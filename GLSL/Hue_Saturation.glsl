//author: yixuan, lu

//Photoshop operation: hue and saturaion

//hue range: [-180, 180]/360
//saturation range: [-100， 100]/100
//luminosity range: [-100, 100]/100
//hsl=(hue, saturation, luminosity)

vec4 HueSaturation(vec4 C, vec3 hsl)
{
	if(hsl.b>0.0)
		C=C*(1.0-hsl.b)+vec4(hsl.b);
	else
		C=C*(1.0+hsl.b);
	vec3 CC=RGB2HSL(C.rgb);
	CC.r+=hsl.r;
	vec3 C_new=HSL2RGB(CC);
	if(hsl.g>=0.0)
	{
		float alpha=(hsl.g+CC.g>=1.0)?max(CC.g,0.01):1.0-hsl.g;
		alpha = 1.0/alpha-1.0;
		C_new=C_new+(C_new-vec3(CC.b))*alpha;
	}
	else
		C_new=vec3(CC.b)+(C_new-CC.b)*(1.0+hsl.g);
	return vec4(clamp(C_new, vec3(0.0), vec3(1.0)),1.0);
}