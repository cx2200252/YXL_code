//author: yixuan, lu

vec3 RGB2HSL(vec3 rgb)
{
	float _min=min(rgb.r, min(rgb.g, rgb.b));
	float _max=max(rgb.r, max(rgb.g, rgb.b));
	if(_max-_min<1e-6)
		return vec3(0.0, 0.0, _min);
	vec3 res;
	if(_max-rgb.r<1e-6)
		if(rgb.g>=rgb.b)
			res.r=60.0*(rgb.g-rgb.b)/(_max-_min);
		else
			res.r=60.0*(rgb.g-rgb.b)/(_max-_min)+360.0;
	else if(_max-rgb.g<1e-6)
		res.r=60.0*(rgb.b-rgb.r)/(_max-_min)+120.0;
	else
		res.r=60.0*(rgb.r-rgb.g)/(_max-_min)+240.0;
	res.r*=1.0/360.0;
	res.b=0.5*(_min+_max);
	if(res.b<=0.5)
		res.g=(_max-_min)/(2.0*res.b);
	else
		res.g=(_max-_min)/(2.0-2.0*res.b);
	return res;
}
vec3 HSL2RGB(vec3 hsl)
{
	if(hsl.g<=0.0)
		return vec3(hsl.b);
	float q;
	if(hsl.b<0.5)
		q=hsl.b*(1.0+hsl.g);
	else
		q=hsl.b+hsl.g-hsl.g*hsl.b;
	float p=2.0*hsl.b-q;
	vec3 res;
	float t=hsl.r+1.0/3.0;
	t=(t>1.0)?(t-1.0):(t<0.0?t+1.0:t);
	if(t<1.0/6.0)
		res.r=p+(q-p)*6.0*t;
	else if(t<0.5)
		res.r=q;
	else if(t<2.0/3.0)
		res.r=p+(q-p)*6.0*(2.0/3.0-t);
	else
		res.r=p;
	t=hsl.r;
	t=(t>1.0)?(t-1.0):(t<0.0?t+1.0:t);
	if(t<1.0/6.0)
		res.g=p+(q-p)*6.0*t;
	else if(t<0.5)
		res.g=q;
	else if(t<2.0/3.0)
		res.g=p+(q-p)*6.0*(2.0/3.0-t);
	else
		res.g=p;
	t=hsl.r-1.0/3.0;
	t=(t>1.0)?(t-1.0):(t<0.0?t+1.0:t);
	if(t<1.0/6.0)
		res.b=p+(q-p)*6.0*t;
	else if(t<0.5)
		res.b=q;
	else if(t<2.0/3.0)
		res.b=p+(q-p)*6.0*(2.0/3.0-t);
	else
		res.b=p;
	return res;
}