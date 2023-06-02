#version 330 core
out vec4 fragColor;

// This helper function returns 1.0 if the current pixel is on a grid line, 0.0 otherwise
vec3 IsGridLine(vec2 fragCoord, vec2 gridSize) {
  // fragCoord is an input to the shader, it defines the pixel co-ordinate of the current pixel
  vec2 vScreenPixelCoordinate = fragCoord.xy;

  // Get a value in the range 0->1 based on where we are in each grid square
  // fract() returns the fractional part of the value and throws away the whole number part
  // This helpfully wraps numbers around in the 0->1 range
  vec2 vGridSquareCoords = fract(vScreenPixelCoordinate / gridSize);

  // Convert the 0->1 co-ordinates of where we are within the grid square
  // back into pixel co-ordinates within the grid square
  vec2 vGridSquarePixelCoords = vGridSquareCoords * gridSize;

  // step() returns 0.0 if the second parmeter is less than the first, 1.0 otherwise
  // so we get 1.0 if we are on a grid line, 0.0 otherwise
  vec2 vIsGridLine = step(vGridSquarePixelCoords, vec2(1.0));

  // Combine the x and y gridlines by taking the maximum of the two values
  vec3 fIsGridLine = vec3(max(vIsGridLine.x, vIsGridLine.y));

  // return the result
  return fIsGridLine;
}

void main() {
  vec3 vResult = vec3(0.116, 0.2, 0.25);

  vResult += (IsGridLine(gl_FragCoord.xy, vec2(64, 64)) * 0.45);
  vResult += (IsGridLine(gl_FragCoord.xy, vec2(32, 32)) * 0.05);

  fragColor = vec4(vResult, 1.0);
}
