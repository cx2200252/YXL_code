//author: yixuan, lu

vec3 RGB2HSV(vec3 rgb)
{
	float _min=min(rgb.r, min(rgb.g, rgb.b));
	float _max=max(rgb.r, max(rgb.g, rgb.b));
	vec3 res;
	if(_max-_min<1e-6)
		res.r=0.0;
	else if(_max-rgb.r<1e-6)
		if(rgb.g>=rgb.b)
			res.r=60.0*(rgb.g-rgb.b)/(_max-_min);
		else
			res.r=60.0*(rgb.g-rgb.b)/(_max-_min)+360.0;
	else if(_max-rgb.g<1e-6)
		res.r=60.0*(rgb.b-rgb.r)/(_max-_min)+120.0;
	else
		res.r=60.0*(rgb.r-rgb.g)/(_max-_min)+240.0;
	res.r*=1.0/360.0;

	res.g=(_max<=0.0)? 0.0 : (_max-_min)/_max;
	res.b=_max;

	return res;
}
vec3 HSV2RGB(vec3 hsv)
{
	int hi=int(hsv.r*6.0);
	float f=hsv.r*6.0-float(hi);
	float p=hsv.b*(1.0-hsv.g);
	float q=hsv.b*(1.0-hsv.g*f);
	float t=hsv.b*(1.0-hsv.g*(1.0-f));
	if(0==hi)
		return vec3(hsv.b, t, p);
	if(1==hi)
		return vec3(q, hsv.b, p);
	if(2==hi)
		return vec3(p, hsv.b, t);
	if(3==hi)
		return vec3(p, q, hsv.b);
	if(4==hi)
		return vec3(t, p, hsv.b);
	return vec3(hsv.b, p, q);
}