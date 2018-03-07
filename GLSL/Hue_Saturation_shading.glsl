//author: yixuan, lu

//Photoshop operation: hue and saturaion with shading

//hue range: [0, 360]/360
//saturation range: [0， 100]/100
//luminosity range: [-100, 100]/100
//hsl=(hue, saturation, luminosity)

vec4 HueSaturationShading(vec4 C, vec3 hsl)
{
	if(hsl.b>0.0)
		C=C*(1.0-hsl.b)+vec4(hsl.b);
	else
		C=C*(1.0+hsl.b);
	vec3 CC=RGB2HSL(C.rgb);
	CC.rg=hsl.rg;
	vec3 C_new=HSL2RGB(CC);
	return vec4(clamp(C_new, vec3(0.0), vec3(1.0)),1.0);
}