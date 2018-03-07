//author: yixuan, lu

//Photoshop operation: black and white

//all range: [-200, 300]/100
//a=(red, green, blue)
//b=(cyan, magenta, yellow)

vec4 RGB2GRAY(vec4 C, vec3 a, vec3 b)
{
	vec3 c;
	vec2 d;
	if(C.r>C.g&&C.r>C.b)
	{
		if(C.g>C.b)
		{	
			c=C.rgb;
			d=vec2(a.r, b.b);
		}
		else
		{
			c=C.rbg;
			d=vec2(a.r, b.g);
		}
	}
	else if(C.g>C.r&&C.g>C.b)
	{
		if(C.r>C.b)
		{
			c=C.grb;
			d=vec2(a.g, b.b);
		}
		else
		{
			c=C.gbr;
			d=vec2(a.g, b.r);
		}
	}
	else
	{
		if(C.r>C.g)
		{
			c=C.brg;
			d=vec2(a.b, b.g);
		}
		else
		{
			c=C.bgr;
			d=vec2(a.b, b.r);
		}
	}
	return vec4(clamp(vec3((c.r-c.g)*d.r+(c.g-c.b)*d.g+c.b), vec3(0.0), vec3(1.0)), 1.0);
}