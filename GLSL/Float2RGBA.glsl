// function that encodes float as RBGA
vec4 encode(float val)
{
   vec4 o;
   val *= 255.0;
   o.r = floor(val);
   val = (val - o.r) * 255.0;
   o.g = floor(val);
   val = (val - o.g) * 255.0;
   o.b = floor(val);
   val = (val - o.b) * 255.0;
   o.a = floor(val);
   return o;
}

// function that decodes RGBA into float
float decode(vec4 val)
{
   return val.r/255.0 
      + val.g/(255.0*255.0) 
      + val.b/(255.0*255.0*255.0) 
      + val.a/(255.0*255.0*255.0*255.0);
}