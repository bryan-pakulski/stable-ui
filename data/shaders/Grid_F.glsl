// This shader is copied from Shadertoy https://www.shadertoy.com/view/ctc3zj
// Originally written by lorenzocelli (https://www.shadertoy.com/user/lorenzocelli) under the following license:
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US

#version 330 core
out vec4 fragColor;

uniform float zoom;
uniform vec2 iResolution;
uniform vec2 iPos;

// return true if the number is odd
bool odd(int n) { return n % 2 == 1; }

// return the multiple of delta closest to value
vec2 closestMul(vec2 delta, vec2 value) { return delta * round(value / delta); }

// return the distance of value to the closest multiple of delta
vec2 mulDist(vec2 delta, vec2 value) { return abs(value - closestMul(delta, value)); }

// align the given point to a pixel center if thickness is odd,
// otherwise align the point to a crossing point between pixels
vec2 alignPixel(vec2 point, int thickness) {
  if (odd(thickness))
    return round(point - 0.5) + 0.5;
  else
    return round(point);
}

// return the grid color; see method mainImage for info about parameters
vec4 drawGrid(vec2 position, vec2 origin, vec2 gridSize, vec2 subGridDiv, int thickness, int subThickness,
              float dotRadius, bool squaredDots, vec4 bgColor, vec4 lineColor, vec4 subLineColor, vec4 dotsColor,
              vec4 xAxisColor, vec4 yAxisColor) {
  float th = float(thickness);
  float sth = float(subThickness);

  // align the origin to the closest pixel center
  origin = alignPixel(origin, thickness);

  vec2 relP = position - origin;

  // ---------------------
  // draw the axes
  // ---------------------

  if (abs(relP.y) < th * 0.5) {
    return xAxisColor;
  }

  if (abs(relP.x) < th * 0.5) {
    return yAxisColor;
  }

  vec2 mul = closestMul(gridSize, relP);

  // pixel distance
  vec2 dist = mulDist(gridSize, relP);

  // ---------------------
  // draw the dots
  // ---------------------

  if (dotRadius > 0.0) {
    // antialiasing threshold
    float aa = 1.0;

    float dotDist = squaredDots ? max(dist.x, dist.y) : length(dist);

    // this prevents the dots from being drawn on the axes
    bool drawDots = abs(mul.x) > 0.5 && abs(mul.y) > 0.5;

    if (drawDots && dotDist <= dotRadius + aa) {
      // draw the dots
      float val = max(dotDist - dotRadius, 0.0) / aa;

      return mix(dotsColor, bgColor, val);
    }
  }

  // ---------------------
  // draw the main grid
  // ---------------------

  if (min(dist.x, dist.y) <= th * 0.5) {
    return lineColor;
  }

  // ---------------------
  // draw the sub-grid
  // ---------------------

  // distance from the bottom-left corner of the main
  // grid square that this sub-grid is part of
  dist = abs(relP - gridSize * floor(relP / gridSize));

  vec2 subSize = round(gridSize / subGridDiv);

  if (odd(thickness) != odd(subThickness))
    dist = abs(dist - 0.5);

  vec2 subDist = mulDist(subSize, dist);

  // number of columns and rows
  vec2 rc = round(dist / subSize);

  // extra pixels that we need to add to the last row/column
  vec2 extra = gridSize - subSize * subGridDiv;

  if (rc.x == subGridDiv.x) // last column
    subDist.x += extra.x;

  if (rc.y == subGridDiv.y) // last row
    subDist.y += extra.y;

  if (min(subDist.x, subDist.y) <= sth * 0.5) {
    return subLineColor;
  }

  // ---------------------

  return bgColor;
}

void main() {
  // ---------------------
  // grid parameters
  // ---------------------

  // origin of the grid (crossing point of the axes)
  vec2 origin = iResolution.xy / 2.0;

  // thickness for the main grid lines
  int thickness = 1;

  // thickness for the sub-grid lines
  // note: if thickness is odd and subThickness is even or
  // vice-versa, there may be a small misalignment
  int subThickness = 1;

  // when greater than zero, draw dots or squares in the
  // crossing points of the main grid
  float dotRadius = 0.0;

  // when true, draw squared dots instead of circles
  bool squaredDots = false;

  // size of the grid
  vec2 gridSize = vec2(100);

  // number of subdivisions of the sub-grid
  vec2 subGridDiv = vec2(3, 3);

  // ---------------------
  // grid colors
  // ---------------------

  // background color
  vec4 bgColor = vec4(0.2, 0.2, 0.2, 1.0);

  // color of the main grid lines
  vec4 lineColor = vec4(0.4, 0.4, 0.4, 1.0);

  // color of the sub-grid lines
  vec4 subLineColor = vec4(0.25, 0.25, 0.25, 1.0);

  // color of the dots (when enabled)
  vec4 dotsColor = lineColor;

  // color of the x axis (currently red)
  vec4 xAxisColor = vec4(212.0 / 255.0, 28.0 / 255.0, 15.0 / 255.0, 1.0);

  // color of the y axis (currently green)
  vec4 yAxisColor = vec4(21.0 / 255.0, 191.0 / 255.0, 83.0 / 255.0, 1.0);

  // ---------------------

  // Origin moves based on camera position, we center at the screen hence the use of iResolution/2
  origin = vec2(iPos.x + (iResolution.x / 2), iPos.y + (iResolution.y / 2));

  // Zoom
  int minZoom = 100;
  int maxZoom = 1000;
  float zoomConv = (((zoom - 0.05) * 100) / 4.95) / 100;
  int size = int(round(mix(float(maxZoom), float(minZoom), zoomConv)));
  gridSize = vec2(size);
  subLineColor = mix(bgColor, lineColor, mix(0.0, 1.0, zoomConv));

  fragColor = drawGrid(gl_FragCoord.xy, origin, gridSize, subGridDiv, thickness, subThickness, dotRadius, squaredDots,
                       bgColor, lineColor, subLineColor, dotsColor, xAxisColor, yAxisColor);
}