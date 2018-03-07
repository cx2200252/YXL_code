//author: yixuan, lu

//Photoshop operation: selective color (absolute mode)

//something wrong...
//cannot use

///*
vec4 SelectiveColorAbsolute(vec4 C, mat4 AA, mat4 BB, vec4 CC)
{
	vec3 sorted=C.rgb;
	if(sorted.y>sorted.x)
		sorted.xy=sorted.yx;
	if(sorted.z>sorted.y)
		sorted.yz=sorted.zy;
	if(sorted.y>sorted.x)
		sorted.xy=sorted.yx;
	float c[9]=float[9](0.0);
	c[0]=C.r-sorted.z;
	c[2]=C.g-sorted.z;
	c[4]=C.b-sorted.z;
	if(sorted.z<=0.5)
	{
		c[6]=0.0;
		c[7]=sorted.z*2.0;
		c[8]=1.0-c[7]*1.67;
	}
	else
	{
		c[6]=sorted.z*2.0-1.0;
		c[7]=1.0-c[6];
		c[8]=0.0;
	}
	if(c[0]>0.0&&c[4]>0.0)
	{
		c[5]=min(c[0], c[4]);
		c[0]-=c[5];
		c[4]-=c[5];
	}
	if(c[0]>0.0&&c[2]>0.0)
	{
		c[1]=min(c[0], c[2]);
		c[0]-=c[1];
		c[2]-=c[1];
	}
	if(c[4]>0.0&&c[2]>0.0)
	{
		c[3]=min(c[4], c[2]);
		c[4]-=c[3];
		c[2]-=c[3];
	}
	vec3 delta=vec3(0.0);
	vec3 ratio, ratio_pos=C.rgb, ratio_neg=C.rgb-1.0;
	for(int i=0;i<4;++i)
	{
		if(c[i]>0.0&&AA[i][0]>=-1.0)
		{
			ratio=clamp(vec3(AA[i][0],AA[i][1],AA[i][2])+AA[i][3],ratio_neg, ratio_pos);
			delta-=c[i]*ratio;
		}
		if(c[i+4]>0.0&&BB[i][0]>=-1.0)
		{
			ratio=clamp(vec3(BB[i][0],BB[i][1],BB[i][2])+BB[i][3],ratio_neg, ratio_pos);
			delta-=c[i+4]*ratio;
		}
	}
	if(c[8]>0.0&&CC.x>=-1.0)
	{
		ratio=clamp(vec3(CC.x,CC.y,CC.z)+CC.w,ratio_neg, ratio_pos);
		delta-=c[8]*ratio;
	}
	return vec4(clamp(C.rgb+delta, vec3(0.0), vec3(1.0)), 1.0);
}
//*/