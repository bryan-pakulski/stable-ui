#version 330 core

#define ITERATIONS 18
#define FORMUPARAMS 0.53

#define VOLSTEPS 20
#define STEPSIZE 0.1

#define ZOOM 6.100
#define TILE 1.850
#define SPEED 0.0001

#define BRIGHTNESS 0.00015
#define DARKMATTER 0.800
#define DISTFADING 0.730
#define SATURATION 0.250

uniform vec2 iResolution;
uniform vec2 iMouse;
uniform float iTime;

out vec4 fragColor;

// Grid rendering
float miv(vec2 a){return min(a.y,a.x);}//return max domain of vector.
float miv(vec3 a){return min(a.z,miv(a.xy));}
float miv(vec4 a){return min(miv(a.zw),miv(a.xy));}
#define mav(a) -miv(-a)
#define grid(u) mav(abs(fract(u)*2.-1.))

void main() {
  // GET COORDS AND DIRECTIONS
  vec2 uv = (gl_FragCoord.xy / iResolution) - 0.5;
  uv.y *= iResolution.y / iResolution.x;
  vec3 dir = vec3(uv * ZOOM, 1.0);
  float time = iTime * SPEED + 0.25;

  // COLOR
  vec4 color = vec4(uv, 0.5 + 0.5 * sin(iTime), 1.0);

  // mouse rotation
  float a1 = 0.5 + iMouse.x / iResolution.x * 2.;
  float a2 = 0.8 + iMouse.y / iResolution.y * 2.;
  mat2 rot1 = mat2(cos(a1), sin(a1), -sin(a1), cos(a1));
  mat2 rot2 = mat2(cos(a2), sin(a2), -sin(a2), cos(a2));
  dir.xz *= rot1;
  dir.xy *= rot2;
  vec3 from = vec3(1., .5, 0.5);
  from += vec3(-time * 2.0, time, -2.0);
  from.xz *= rot1;
  from.xy *= rot2;

  // volumetric rendering
  float s = 0.1, fade = 1.0;
  vec3 v = vec3(0.0);
  for (int r = 0; r < VOLSTEPS; r++) {
    vec3 p = from + s * dir * 0.5;
    p = abs(vec3(TILE) - mod(p, vec3(TILE * 2.0))); // tiling fold
    float pa, a = pa = 0.0;
    for (int i = 0; i < ITERATIONS; i++) {
      p = abs(p) / dot(p, p) - FORMUPARAMS; // the magic formula
      a += abs(length(p) - pa);             // absolute sum of average change
      pa = length(p);
    }
    float dm = max(0.0, DARKMATTER - a * a * 0.001); // dark matter
    a *= a * a;                                      // add contrast
    if (r > 6)
      fade *= 1.0 - dm; // dark matter, don't render near
    v += fade;
    v += vec3(s, s * s, s * s * s * s) * a * BRIGHTNESS * fade; // coloring based on distance
    fade *= DISTFADING;                                         // distance fading
    s += STEPSIZE;
  }

  v = mix(vec3(length(v)), v, SATURATION); // color adjust
  color = vec4(v * 0.01, 1.0);

  vec4 cold = vec4(0.7, 1.8, 2.9, 1.0);
  vec4 warm = vec4(3.7, 2.5, 0.9, 1.0);

  float colorMix = 0.5f;
  color *= mix(cold, warm, colorMix);

  fragColor = vec4(color.r, color.g, color.b, 1.0);

  w// 2D Grid
  float zoom =10.;//2d scaling
  float thick=2.5;//line thickness in screenspace pixels (hairline)
  
  float gridLineThickness = zoom*thick/iResolution.y;        
  fragColor += vec4(0,smoothstep(1.-gridLineThickness,1.,grid(uv*zoom)), 0, 1);
}