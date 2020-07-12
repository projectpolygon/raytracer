# raytracer
ProjectPolygon Raytracer

[![CodeFactor](https://www.codefactor.io/repository/github/projectpolygon/raytracer/badge/master?s=f87b3bf21de25a2ad3cbfb1c2882c2bb7c40d0bf)](https://www.codefactor.io/repository/github/projectpolygon/raytracer/overview/master)

## Objects
Objects can be rendered by adding the following objects to the "objects" list in the taskfile
### Spheres
```
{
	"type": "sphere",
	"centre": [ 0.0, 0.0, 150.0 ],
	"radius": 60.0,
	"material": {
		    "type": "matte",
		    "diffuse": 1.0,
		    "colour": [ 0.0, 0.0, 1.0 ]
	}
}
```

### Torus
```
{
	"type": "torus",
	"centre": [ 0.0, 0.0, 150.0 ],
	"inner_radius": 60.0,
	"outer_radius": 60.0,
	"material": {
		"type": "matte",
		"diffuse": 1.0,
		"colour": [ 1.0, 0.0, 0.0 ]
	}
}
```

### Triangle
```
{
	"type": "triangle",
	"points": [
		  [ 0.0, 0.0, 0.0 ],
		  [ 200.0, 0.0, 1.0 ],
		  [ 0.0, 200.0, 0.0 ]
	],
	"position": [ 300.0, 0.0, 0.0 ],
	"material": {
		    "type": "matte",
		    "diffuse": 1.0,
		    "colour": [ 0.0, 0.0, 1.0 ]
	}
}
```

### Mesh
```
{
	"type": "mesh",
	"object_file": "suzanne.obj",
	"material_file": "",
	"position": [ 0.0, 0.0, 0.0 ],
	"scale": [100.0, 100.0, 100.0],
	"material": {
		"type": "matte",
		"diffuse": 1.0,
		"colour": [ 0.0, 0.0, 1.0 ]
	}
}
```
