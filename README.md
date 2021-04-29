Advanced Shaders 2021

*THE DEFAULT SHADER IS A TESSELATION SHADER WITH PN TRIANGLES*
*(CHANGE THIS WITH KEYS 1-3)*
 
---------------------------------- Key Controls ----------------------------------
- Esc = Close program.
- W = Move camera forwards.
- A = Move camera left.
- S = Move camera backwards.
- D = Move camera right.

- 1 = Plain shader with no lighting model. 
- 2 = Tesselation shader with phong lighting. 
- 3 = Tesselation shader with phong lighting and PN triangles. (This is the default shader on program start).

- < = Use colour frame buffer texture.
- > = Use depth frame buffer texture.

- F1 = Reset post processing effects to none.
- F2 = Set scene colours to negative in post processing shader.
- F3 = Set scene colours to black & white in post processing shader.

- Up Arrow - Decrease depth FAR PLANE if depth shading is being used.
- Down Arrow - Increase depth FAR PLANE if depth shading is being used.
- Left Arrow - Decrease depth NEAR PLANE if depth shading is being used.
- Right Arrow - Increase depth NEAR PLANE if depth shading is being used.

- M = Fill mesh render.
- L = Wire mesh render. (Will not have much effect due to post processing frame buffer meaning only a texture quad is rendered).


~ ~ ~ ~ ~ FOLLOWING ONLY APPLY TO TESSELATION SHADERS ~ ~ ~ ~ ~
- Keypad Add (+) = Increase heightmap scale. (For shaders 2 & 3).
- Keypad Subtract (-)  = Decrease heightmap scale. (For shaders 2 & 3).

- T = Step tesselation calculation. (For shaders 2 & 3).
- E = Exponential tesselation calculation. (For shaders 2 & 3).

- F = Add fog to scene. (For shaders 2 & 3).
- C = Clear for from scene. (For shaders 2 & 3).



