gl = Module("gl");

Shader = Class() :: @{
	$__initialize = @(program) {
		$program = program;
	};
	$setup = @(uniforms, attributes) {};
	$teardown = @{};
	$__call = @(uniforms, attributes, mode, offset, count) {
		gl.use_program($program);
		$setup(uniforms, attributes);
		gl.draw_arrays(mode, offset, count);
		$teardown();
		gl.use_program(null);
		gl.bind_buffer(gl.ARRAY_BUFFER, null);
	};
};

WithVertex = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$projection = $program.get_uniform_location("projection");
		$vertex_matrix = $program.get_uniform_location("vertexMatrix");
		$vertex = $program.get_attrib_location("vertex");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		$projection.matrix4fv(true, uniforms.projection);
		$vertex_matrix.matrix4fv(true, uniforms.vertex);
		gl.enable_vertex_attrib_array($vertex);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.vertices);
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 0, 0);
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($vertex);
	};
};

WithNormal = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$normal = $program.get_attrib_location("normal");
		$normal_matrix = $program.get_uniform_location("normalMatrix");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		if (attributes.normals !== null) {
			gl.enable_vertex_attrib_array($normal);
			gl.bind_buffer(gl.ARRAY_BUFFER, attributes.normals);
			gl.vertex_attrib_pointer($normal, 3, gl.FLOAT, false, 0, 0);
			$normal_matrix.matrix3fv(true, uniforms.normal);
		}
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($normal);
	};
};

WithColor = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$color = $program.get_uniform_location("color");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		color = uniforms.color;
		$color.uniform4f(color.x, color.y, color.z, color.w);
	};
};

WithTexture = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$texcoord = $program.get_attrib_location("texcoord");
		$color = $program.get_uniform_location("color");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		gl.enable_vertex_attrib_array($texcoord);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.texcoords);
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, 0, 0);
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, uniforms.color);
		$color.uniform1i(0);
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($texcoord);
		gl.bind_texture(gl.TEXTURE_2D, null);
	};
};

WithDiffuseColor = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$diffuse = $program.get_uniform_location("diffuse");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		diffuse = uniforms.diffuse;
		$diffuse.uniform4f(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
	};
};

WithDiffuseTexture = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$texcoord = $program.get_attrib_location("texcoord");
		$diffuse = $program.get_uniform_location("diffuse");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		gl.enable_vertex_attrib_array($texcoord);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.texcoords);
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, 0, 0);
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, uniforms.diffuse);
		$diffuse.uniform1i(0);
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($texcoord);
		gl.bind_texture(gl.TEXTURE_2D, null);
	};
};

WithSpecular = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$specular = $program.get_uniform_location("specular");
		$shininess = $program.get_uniform_location("shininess");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		specular = uniforms.specular;
		$specular.uniform4f(specular.x, specular.y, specular.z, specular.w);
		$shininess.uniform1f(uniforms.shininess);
	};
};

WithRefraction = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$refraction = $program.get_uniform_location("refraction");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		$refraction.uniform1f(uniforms.refraction);
	};
};

WithSkin = @(n) @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$projection = $program.get_uniform_location("projection");
		$vertex_matrices = $program.get_uniform_location("vertexMatrices");
		$vertex = $program.get_attrib_location("vertex");
		$joints = [];
		$weights = [];
		for (i = 0; i < n; i = i + 1) {
			$joints.push($program.get_attrib_location("joint" + i));
			$weights.push($program.get_attrib_location("weight" + i));
		}
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		$projection.matrix4fv(true, uniforms.projection);
		$vertex_matrices.matrix4fv(true, uniforms.vertices);
		gl.enable_vertex_attrib_array($vertex);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.vertices);
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 0, 0);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.joints);
		for (i = 0; i < n; i = i + 1) {
			gl.enable_vertex_attrib_array($joints[i]);
			gl.vertex_attrib_pointer($joints[i], 1, gl.FLOAT, false, n * gl.Float32Array.BYTES_PER_ELEMENT, i * gl.Float32Array.BYTES_PER_ELEMENT);
		}
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.weights);
		for (i = 0; i < n; i = i + 1) {
			gl.enable_vertex_attrib_array($weights[i]);
			gl.vertex_attrib_pointer($weights[i], 1, gl.FLOAT, false, n * gl.Float32Array.BYTES_PER_ELEMENT, i * gl.Float32Array.BYTES_PER_ELEMENT);
		}
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($vertex);
		for (i = 0; i < n; i = i + 1) {
			gl.disable_vertex_attrib_array($joints[i]);
			gl.disable_vertex_attrib_array($weights[i]);
		}
	};
};

WithSkinNormal = @{
	super__initialize = $__initialize;
	$__initialize = @(program) {
		super__initialize[$](program);
		$normal = $program.get_attrib_location("normal");
	};
	super__setup = $setup;
	$setup = @(uniforms, attributes) {
		super__setup[$](uniforms, attributes);
		gl.enable_vertex_attrib_array($normal);
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes.normals);
		gl.vertex_attrib_pointer($normal, 3, gl.FLOAT, false, 0, 0);
	};
	super__teardown = $teardown;
	$teardown = @{
		super__teardown[$]();
		gl.disable_vertex_attrib_array($normal);
	};
};

ColorShader = Class(Shader) :: WithVertex :: WithColor;
TextureShader = Class(Shader) :: WithVertex :: WithTexture;
DiffuseColorShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseColor;
DiffuseTextureShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseTexture;
DiffuseColorSpecularShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseColor :: WithSpecular;
DiffuseTextureSpecularShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseTexture :: WithSpecular;
DiffuseColorSpecularRefractionShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseColor :: WithSpecular :: WithRefraction;
DiffuseTextureSpecularRefractionShader = Class(Shader) :: WithVertex :: WithNormal :: WithColor :: WithDiffuseTexture :: WithSpecular :: WithRefraction;

compile = @(type, source) {
	shader = gl.Shader(type);
	shader.source(source);
	shader.compile();
	if (shader.get_parameteri(gl.COMPILE_STATUS) == gl.FALSE) throw Throwable(source + shader.get_info_log());
	shader;
};
$compile = compile;

link = @(vshader, fshader) {
	program = gl.Program();
	program.attach_shader(vshader);
	program.attach_shader(fshader);
	program.link();
	if (program.get_parameteri(gl.LINK_STATUS) == gl.FALSE) throw Throwable(program.get_info_log());
	program;
};
$link = link;

$__call = Class() :: @{
	$__initialize = @{
		$_vertex_shader = null;
		$_vertex_shader_texture = null;
		$_vertex_shader_normal = null;
		$_vertex_shader_normal_texture = null;
		$_constant_shader_color = null;
		$_constant_shader_texture = null;
		$_constant_color = null;
		$_constant_texture = null;
		$_blinn_shader_color = null;
		$_blinn_shader_texture = null;
		$_blinn_color = null;
		$_blinn_texture = null;
		$_lambert_shader_color = null;
		$_lambert_shader_texture = null;
		$_lambert_color = null;
		$_lambert_texture = null;
		$_phong_shader_color = null;
		$_phong_shader_texture = null;
		$_phong_color = null;
		$_phong_texture = null;
		$_skins_shader_normal = {};
		$_skins_shader_normal_texture = {};
		$_SkinColorShaders = {};
		$_SkinDiffuseColorShaders = {};
		$_SkinDiffuseColorSpecularShaders = {};
		$_SkinDiffuseColorSpecularRefractionShaders = {};
		$_skins_color = {};
		$_SkinTextureShaders = {};
		$_SkinDiffuseTextureShaders = {};
		$_SkinDiffuseTextureSpecularShaders = {};
		$_SkinDiffuseTextureSpecularRefractionShaders = {};
		$_skins_texture = {};
	};
	vertex_shader = @(defines) {
		compile(gl.VERTEX_SHADER, defines + "
uniform mat4 projection;
uniform mat4 vertexMatrix;
attribute vec3 vertex;

void main()
{
	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));
}
");
	};
	$vertex_shader = @{
		if ($_vertex_shader === null) $_vertex_shader = vertex_shader("");
		$_vertex_shader;
	};
	$vertex_shader_texture = @{
		if ($_vertex_shader_texture === null) $_vertex_shader_texture = vertex_shader("#define USE_TEXTURE");
		$_vertex_shader_texture;
	};
	vertex_shader_normal = @(defines) {
		compile(gl.VERTEX_SHADER, defines + "
uniform mat4 projection;
uniform mat4 vertexMatrix;
attribute vec3 vertex;
attribute vec3 normal;
uniform mat3 normalMatrix;
varying vec3 varyingNormal;
#ifdef USE_TEXTURE
attribute vec2 texcoord;
varying vec2 varyingTexcoord;
#endif

void main()
{
	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));
	varyingNormal = normalize(normal * normalMatrix);
#ifdef USE_TEXTURE
	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
#endif
}
");
	};
	$vertex_shader_normal = @{
		if ($_vertex_shader_normal === null) $_vertex_shader_normal = vertex_shader_normal("");
		$_vertex_shader_normal;
	};
	$vertex_shader_normal_texture = @{
		if ($_vertex_shader_normal_texture === null) $_vertex_shader_normal_texture = vertex_shader_normal("#define USE_TEXTURE");
		$_vertex_shader_normal_texture;
	};
	constant_shader = @(defines) {
		compile(gl.FRAGMENT_SHADER, defines + "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

#ifdef USE_TEXTURE
uniform sampler2D color;
varying vec2 varyingTexcoord;
#else
uniform vec4 color;
#endif

void main()
{
#ifdef USE_TEXTURE
	vec4 sample = texture2D(color, varyingTexcoord);
	if (sample.a < 0.5) discard;
	gl_FragColor = sample;
#else
	gl_FragColor = color;
#endif
}
");
	};
	$constant_shader_color = @{
		if ($_constant_shader_color === null) $_constant_shader_color = constant_shader("");
		$_constant_shader_color;
	};
	$constant_shader_texture = @{
		if ($_constant_shader_texture === null) $_constant_shader_texture = constant_shader("#define USE_TEXTURE");
		$_constant_shader_texture;
	};
	$constant_color = @{
		if ($_constant_color === null) $_constant_color = ColorShader(link($vertex_shader(), $constant_shader_color()));
		$_constant_color;
	};
	$constant_texture = @{
		if ($_constant_texture === null) $_constant_texture = TextureShader(link($vertex_shader_texture(), $constant_shader_texture()));
		$_constant_texture;
	};
	blinn_shader = @(defines) {
		compile(gl.FRAGMENT_SHADER, defines + "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform vec4 color;
varying vec3 varyingNormal;
#ifdef USE_TEXTURE
uniform sampler2D diffuse;
varying vec2 varyingTexcoord;
#else
uniform vec4 diffuse;
#endif
uniform vec4 specular;
uniform float shininess;
uniform float refraction;

void main()
{
#ifdef USE_TEXTURE
	vec4 sample = texture2D(diffuse, varyingTexcoord);
	if (sample.a < 0.5) discard;
#endif
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
	vec3 eye = vec3(0.0, 0.0, 1.0);
	vec3 h = (light + eye) * 0.5;
	float nh = dot(varyingNormal, h);
	float c2 = shininess * shininess;
	float D = c2 / (nh * nh * (c2 - 1.0) + 1.0);
	float eh = dot(eye, h);
	float ne = dot(varyingNormal, eye);
	float nl = dot(varyingNormal, light);
	float G = min(1.0, 2.0 * nh * min(ne, nl) / eh);
	float g = sqrt(refraction * refraction + eh * eh - 1.0);
	float gpc = g + eh;
	float gmc = g - eh;
	float cgpcm1 = eh * gpc - 1.0;
	float cgmcp1 = eh * gmc + 1.0;
	float F = gmc * gmc * (1.0 + cgpcm1 * cgpcm1 / (cgmcp1 * cgmcp1)) * 0.5 / (gpc * gpc);
#ifdef USE_TEXTURE
	gl_FragColor = color + sample
#else
	gl_FragColor = color + diffuse
#endif
	* max(dot(varyingNormal, light), 0.0) + specular * max(D * D * G * F / ne, 0.0);
}
");
	};
	$blinn_shader_color = @{
		if ($_blinn_shader_color === null) $_blinn_shader_color = blinn_shader("");
		$_blinn_shader_color;
	};
	$blinn_shader_texture = @{
		if ($_blinn_shader_texture === null) $_blinn_shader_texture = blinn_shader("#define USE_TEXTURE");
		$_blinn_shader_texture;
	};
	$blinn_color = @{
		if ($_blinn_color === null) $_blinn_color = DiffuseColorSpecularRefractionShader(link($vertex_shader_normal(), $blinn_shader_color()));
		$_blinn_color;
	};
	$blinn_texture = @{
		if ($_blinn_texture === null) $_blinn_texture = DiffuseTextureSpecularRefractionShader(link($vertex_shader_normal_texture(), $blinn_shader_texture()));
		$_blinn_texture;
	};
	lambert_shader = @(defines) {
		compile(gl.FRAGMENT_SHADER, defines + "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform vec4 color;
varying vec3 varyingNormal;
#ifdef USE_TEXTURE
uniform sampler2D diffuse;
varying vec2 varyingTexcoord;
#else
uniform vec4 diffuse;
#endif

void main()
{
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
#ifdef USE_TEXTURE
	vec4 sample = texture2D(diffuse, varyingTexcoord);
	if (sample.a < 0.5) discard;
	gl_FragColor = color + sample * max(dot(varyingNormal, light), 0.0);
#else
	gl_FragColor = color + diffuse * max(dot(varyingNormal, light), 0.0);
#endif
}
");
	};
	$lambert_shader_color = @{
		if ($_lambert_shader_color === null) $_lambert_shader_color = lambert_shader("");
		$_lambert_shader_color;
	};
	$lambert_shader_texture = @{
		if ($_lambert_shader_texture === null) $_lambert_shader_texture = lambert_shader("#define USE_TEXTURE");
		$_lambert_shader_texture;
	};
	$lambert_color = @{
		if ($_lambert_color === null) $_lambert_color = DiffuseColorShader(link($vertex_shader_normal(), $lambert_shader_color()));
		$_lambert_color;
	};
	$lambert_texture = @{
		if ($_lambert_texture === null) $_lambert_texture = DiffuseTextureShader(link($vertex_shader_normal_texture(), $lambert_shader_texture()));
		$_lambert_texture;
	};
	phong_shader = @(defines) {
		compile(gl.FRAGMENT_SHADER, defines + "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform vec4 color;
varying vec3 varyingNormal;
#ifdef USE_TEXTURE
uniform sampler2D diffuse;
varying vec2 varyingTexcoord;
#else
uniform vec4 diffuse;
#endif
uniform vec4 specular;
uniform float shininess;

void main()
{
#ifdef USE_TEXTURE
	vec4 sample = texture2D(diffuse, varyingTexcoord);
	if (sample.a < 0.5) discard;
#endif
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
	vec3 r = reflect(-light, varyingNormal);
	vec3 eye = vec3(0.0, 0.0, 1.0);
#ifdef USE_TEXTURE
	gl_FragColor = color + sample
#else
	gl_FragColor = color + diffuse
#endif
	* max(dot(varyingNormal, light), 0.0) + specular * pow(max(dot(r, eye), 0.0), shininess);
}
");
	};
	$phong_shader_color = @{
		if ($_phong_shader_color === null) $_phong_shader_color = phong_shader("");
		$_phong_shader_color;
	};
	$phong_shader_texture = @{
		if ($_phong_shader_texture === null) $_phong_shader_texture = phong_shader("#define USE_TEXTURE");
		$_phong_shader_texture;
	};
	$phong_color = @{
		if ($_phong_color === null) $_phong_color = DiffuseColorSpecularShader(link($vertex_shader_normal(), $phong_shader_color()));
		$_phong_color;
	};
	$phong_texture = @{
		if ($_phong_texture === null) $_phong_texture = DiffuseTextureSpecularShader(link($vertex_shader_normal_texture(), $phong_shader_texture()));
		$_phong_texture;
	};
	skin_shader = @(joints, weights, defines) {
		joint = "";
		weight = "";
		vertex = "";
		for (i = 1; i < weights; i = i + 1) {
			joint = joint + "attribute float joint" + i + ";";
			weight = weight + "attribute float weight" + i + ";";
			vertex = vertex + " + vertexMatrices[int(joint" + i + ")] * weight" + i;
		}
		compile(gl.VERTEX_SHADER, defines + "
mat3 invert4to3(inout mat4 m) {
	//float d = m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2] + m[2][0] * m[0][1] * m[1][2] - m[0][0] * m[2][1] * m[1][2] - m[2][0] * m[1][1] * m[0][2] - m[1][0] * m[0][1] * m[2][2];
	return mat3(
		m[1][1] * m[2][2] - m[1][2] * m[2][1],
		m[0][2] * m[2][1] - m[0][1] * m[2][2],
		m[0][1] * m[1][2] - m[0][2] * m[1][1],
		m[1][2] * m[2][0] - m[1][0] * m[2][2],
		m[0][0] * m[2][2] - m[0][2] * m[2][0],
		m[0][2] * m[1][0] - m[0][0] * m[1][2],
		m[1][0] * m[2][1] - m[1][1] * m[2][0],
		m[0][1] * m[2][0] - m[0][0] * m[2][1],
		m[0][0] * m[1][1] - m[0][1] * m[1][0]
	);
}

uniform mat4 projection;
uniform mat4 vertexMatrices[" + joints + "];
attribute vec3 vertex;
attribute vec3 normal;
attribute float joint0;
" + joint + "
attribute float weight0;
" + weight + "
varying vec3 varyingNormal;
#ifdef USE_TEXTURE
attribute vec2 texcoord;
varying vec2 varyingTexcoord;
#endif

void main()
{
	vec4 v = vec4(vertex, 1.0);
	mat4 vm = vertexMatrices[int(joint0)] * weight0" + vertex + ";
	gl_Position = projection * (vm * v);
	varyingNormal = normalize(normal * invert4to3(vm));
#ifdef USE_TEXTURE
	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
#endif
}
");
	};
	$skin_shader_normal = @(joints, weights) {
		key = '(joints, weights);
		if ($_skins_shader_normal.has(key)) return $_skins_shader_normal[key];
		$_skins_shader_normal[key] = skin_shader(joints, weights, "");
	};
	$skin_shader_normal_texture = @(joints, weights) {
		key = '(joints, weights);
		if ($_skins_shader_normal_texture.has(key)) return $_skins_shader_normal_texture[key];
		$_skins_shader_normal_texture[key] = skin_shader(joints, weights, "#define USE_TEXTURE");
	};
	$SkinColorShader = @(n) {
		if ($_SkinColorShaders.has(n)) return $_SkinColorShaders[n];
		$_SkinColorShaders[n] = Class(Shader) :: WithSkin(n) :: WithColor;
	};
	$SkinDiffuseColorShader = @(n) {
		if ($_SkinDiffuseColorShaders.has(n)) return $_SkinDiffuseColorShaders[n];
		$_SkinDiffuseColorShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseColor;
	};
	$SkinDiffuseColorSpecularShader = @(n) {
		if ($_SkinDiffuseColorSpecularShaders.has(n)) return $_SkinDiffuseColorSpecularShaders[n];
		$_SkinDiffuseColorSpecularShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseColor :: WithSpecular;
	};
	$SkinDiffuseColorSpecularRefractionShader = @(n) {
		if ($_SkinDiffuseColorSpecularRefractionShaders.has(n)) return $_SkinDiffuseColorSpecularRefractionShaders[n];
		$_SkinDiffuseColorSpecularRefractionShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseColor :: WithSpecular :: WithRefraction;
	};
	skins_color = Object();
	skins_color.constant = '($SkinColorShader, $constant_shader_color);
	skins_color.blinn = '($SkinDiffuseColorSpecularRefractionShader, $blinn_shader_color);
	skins_color.lambert = '($SkinDiffuseColorShader, $lambert_shader_color);
	skins_color.phong = '($SkinDiffuseColorSpecularShader, $phong_shader_color);
	$skin_color = @(joints, weights, model) {
		key = '(joints, weights, model);
		if ($_skins_color.has(key)) return $_skins_color[key];
		factory = skins_color.(model);
		$_skins_color[key] = factory[0][$](weights)(link($skin_shader_normal(joints, weights), factory[1][$]()));
	};
	$SkinTextureShader = @(n) {
		if ($_SkinTextureShaders.has(n)) return $_SkinTextureShaders[n];
		$_SkinTextureShaders[n] = Class(Shader) :: WithSkin(n) :: WithTexture;
	};
	$SkinDiffuseTextureShader = @(n) {
		if ($_SkinDiffuseTextureShaders.has(n)) return $_SkinDiffuseTextureShaders[n];
		$_SkinDiffuseTextureShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseTexture;
	};
	$SkinDiffuseTextureSpecularShader = @(n) {
		if ($_SkinDiffuseTextureSpecularShaders.has(n)) return $_SkinDiffuseTextureSpecularShaders[n];
		$_SkinDiffuseTextureSpecularShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseTexture :: WithSpecular;
	};
	$SkinDiffuseTextureSpecularRefractionShader = @(n) {
		if ($_SkinDiffuseTextureSpecularRefractionShaders.has(n)) return $_SkinDiffuseTextureSpecularRefractionShaders[n];
		$_SkinDiffuseTextureSpecularRefractionShaders[n] = Class(Shader) :: WithSkin(n) :: WithSkinNormal :: WithColor :: WithDiffuseTexture :: WithSpecular :: WithRefraction;
	};
	skins_texture = Object();
	skins_texture.constant = '($SkinTextureShader, $constant_shader_texture);
	skins_texture.blinn = '($SkinDiffuseTextureSpecularRefractionShader, $blinn_shader_texture);
	skins_texture.lambert = '($SkinDiffuseTextureShader, $lambert_shader_texture);
	skins_texture.phong = '($SkinDiffuseTextureSpecularShader, $phong_shader_texture);
	$skin_texture = @(joints, weights, model) {
		key = '(joints, weights, model);
		if ($_skins_texture.has(key)) return $_skins_texture[key];
		factory = skins_texture.(model);
		$_skins_texture[key] = factory[0][$](weights)(link($skin_shader_normal_texture(joints, weights), factory[1][$]()));
	};
};
