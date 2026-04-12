The original's data regarding coordinates and axes needs some
clarification:

* Map size: u8, u8, small scale
* Map border: u8, small scale
* Height map: u8 x u8 -> u8, height scale
* Polygon paths/water: i32 x i32 x i32, large scale on plane, height scale, rel. to border
* Object coordinates: vec3, large scale on plane, rel. to border

* Small scale: for planes to be scaled by 10 into large scale
* Height scale: small scale, to be drawn at 10/16 of the value, which
  needs to be considered for every translation onto the map.

While the original uses Z as height, for this code, the map is always XZ at
plane, so Y is height. Where feasible (object locations), we swap axes at mesh level.

Rendering is performed in large scale space, whereas height always
remains small scale.
