# NOTES

- On nVidia GPU RTX2070 defining the Texture array as type of sampler2D directly
  won't work (for unknown reasons). It can be done, though, by defining the array
  as uvec2 and later construct a sampler out of it, like so:
  ```glsl
    layout(std430, binding = 5) readonly buffer Textures {
        uvec2 in_Samplers[32];
    };
    
    ...
    
    sampler2D albedo = sampler2D(in_Samplers[material.albedoID]);
  ```

