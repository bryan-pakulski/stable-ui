#version 330 core
out vec4 fragColor;

vec3 IsGridLine(vec2 fragCoord, vec2 gridSize) {
  vec2 vScreenPixelCoordinate = fragCoord.xy;
  vec2 vGridSquareCoords = fract(vScreenPixelCoordinate / gridSize);
  vec2 vGridSquarePixelCoords = vGridSquareCoords * gridSize;
  vec2 vIsGridLine = step(vGridSquarePixelCoords, vec2(1.0));
  vec3 fIsGridLine = vec3(max(vIsGridLine.x, vIsGridLine.y));

  return fIsGridLine;
}

void main() {
  vec3 vResult = vec3(0.116, 0.2, 0.25);

  vResult += (IsGridLine(gl_FragCoord.xy, vec2(64, 64)) * 0.45);
  vResult += (IsGridLine(gl_FragCoord.xy, vec2(32, 32)) * 0.05);

  fragColor = vec4(vResult, 1.0);
}
