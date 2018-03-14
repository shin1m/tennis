gl = Module("gl"

$Uniforms = Class() :: @
	$__initialize = @
		$stride = null
		$texcoords = null
		$projection = null
		$vertex = null
		$vertices = null
		$normal = null
		$color = null
		$diffuse = null
		$specular = null
		$shininess = null
		$refraction = null

MeshShader = Class() :: @
	$__initialize = @(program)
		$program = program
		$projection = $program.get_uniform_location("projection"
		$vertex_matrix = $program.get_uniform_location("vertexMatrix"
		$vertex = $program.get_attrib_location("vertex"
	$call = @(uniforms, mode, offset, count)
		$projection.matrix4fv(false, uniforms.projection
		$vertex_matrix.matrix4fv(false, uniforms.vertex
		gl.enable_vertex_attrib_array($vertex
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, uniforms.stride, 0
		gl.draw_arrays(mode, offset, count
		gl.disable_vertex_attrib_array($vertex
	$__call = @(uniforms, attributes, mode, offset, count)
		gl.use_program($program
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes
		$call(uniforms, mode, offset, count
		gl.use_program(null
		gl.bind_buffer(gl.ARRAY_BUFFER, null

WithNormal = @
	normal_offset = 3 * gl.Float32Array.BYTES_PER_ELEMENT
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$normal = $program.get_attrib_location("normal"
		$normal_matrix = $program.get_uniform_location("normalMatrix"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		gl.enable_vertex_attrib_array($normal
		gl.vertex_attrib_pointer($normal, 3, gl.FLOAT, false, uniforms.stride, normal_offset
		$normal_matrix.matrix3fv(false, uniforms.normal
		super__call[$](uniforms, mode, offset, count
		gl.disable_vertex_attrib_array($normal

WithColor = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$color = $program.get_uniform_location("color"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		color = uniforms.color
		$color.uniform4f(color.x, color.y, color.z, color.w
		super__call[$](uniforms, mode, offset, count

WithTexture = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$texcoord = $program.get_attrib_location("texcoord"
		$color = $program.get_uniform_location("color"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		gl.enable_vertex_attrib_array($texcoord
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, uniforms.stride, uniforms.texcoords
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, uniforms.color
		$color.uniform1i(0
		super__call[$](uniforms, mode, offset, count
		gl.disable_vertex_attrib_array($texcoord
		gl.bind_texture(gl.TEXTURE_2D, null

WithDiffuseColor = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$diffuse = $program.get_uniform_location("diffuse"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		diffuse = uniforms.diffuse
		$diffuse.uniform4f(diffuse.x, diffuse.y, diffuse.z, diffuse.w
		super__call[$](uniforms, mode, offset, count

WithDiffuseTexture = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$texcoord = $program.get_attrib_location("texcoord"
		$diffuse = $program.get_uniform_location("diffuse"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		gl.enable_vertex_attrib_array($texcoord
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, uniforms.stride, uniforms.texcoords
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, uniforms.diffuse
		$diffuse.uniform1i(0
		super__call[$](uniforms, mode, offset, count
		gl.disable_vertex_attrib_array($texcoord
		gl.bind_texture(gl.TEXTURE_2D, null

WithSpecular = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$specular = $program.get_uniform_location("specular"
		$shininess = $program.get_uniform_location("shininess"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		specular = uniforms.specular
		$specular.uniform4f(specular.x, specular.y, specular.z, specular.w
		$shininess.uniform1f(uniforms.shininess
		super__call[$](uniforms, mode, offset, count

WithRefraction = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$refraction = $program.get_uniform_location("refraction"
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		$refraction.uniform1f(uniforms.refraction
		super__call[$](uniforms, mode, offset, count

SkinShader = @(n) Class() :: @
	joints_offset = 3 * gl.Float32Array.BYTES_PER_ELEMENT
	weights_offset = joints_offset + n * gl.Int32Array.BYTES_PER_ELEMENT
	$__initialize = @(program)
		$program = program
		$projection = $program.get_uniform_location("projection"
		$vertex_matrices = $program.get_uniform_location("vertexMatrices"
		$vertex = $program.get_attrib_location("vertex"
		$joints = [
		$weights = [
		for i = 0; i < n; i = i + 1
			$joints.push($program.get_attrib_location("joint" + i
			$weights.push($program.get_attrib_location("weight" + i
	$call = @(uniforms, mode, offset, count)
		$projection.matrix4fv(false, uniforms.projection
		$vertex_matrices.matrix4fv(false, uniforms.vertices
		gl.enable_vertex_attrib_array($vertex
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, uniforms.stride, 0
		for i = 0; i < n; i = i + 1
			gl.enable_vertex_attrib_array($joints[i]
			gl.vertex_attrib_pointer($joints[i], 1, gl.UNSIGNED_INT, false, uniforms.stride, joints_offset + i * gl.Int32Array.BYTES_PER_ELEMENT
			gl.enable_vertex_attrib_array($weights[i]
			gl.vertex_attrib_pointer($weights[i], 1, gl.FLOAT, false, uniforms.stride, weights_offset + i * gl.Float32Array.BYTES_PER_ELEMENT
		gl.draw_arrays(mode, offset, count
		gl.disable_vertex_attrib_array($vertex
		for i = 0; i < n; i = i + 1
			gl.disable_vertex_attrib_array($joints[i]
			gl.disable_vertex_attrib_array($weights[i]
	$__call = @(uniforms, attributes, mode, offset, count)
		gl.use_program($program
		gl.bind_buffer(gl.ARRAY_BUFFER, attributes
		$call(uniforms, mode, offset, count
		gl.use_program(null
		gl.bind_buffer(gl.ARRAY_BUFFER, null

WithSkinNormal = @
	super__initialize = $__initialize
	$__initialize = @(program)
		super__initialize[$](program
		$normal = $program.get_attrib_location("normal"
		$normal_offset = 3 * gl.Float32Array.BYTES_PER_ELEMENT + $joints.size() * gl.Int32Array.BYTES_PER_ELEMENT + $joints.size() * gl.Float32Array.BYTES_PER_ELEMENT
	super__call = $call
	$call = @(uniforms, mode, offset, count)
		gl.enable_vertex_attrib_array($normal
		gl.vertex_attrib_pointer($normal, 3, gl.FLOAT, false, uniforms.stride, $normal_offset
		super__call[$](uniforms, mode, offset, count
		gl.disable_vertex_attrib_array($normal

ColorShader = Class(MeshShader) :: WithColor
TextureShader = Class(MeshShader) :: WithTexture
DiffuseColorShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseColor
DiffuseTextureShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseTexture
DiffuseColorSpecularShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseColor :: WithSpecular
DiffuseTextureSpecularShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseTexture :: WithSpecular
DiffuseColorSpecularRefractionShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseColor :: WithSpecular :: WithRefraction
DiffuseTextureSpecularRefractionShader = Class(MeshShader) :: WithNormal :: WithColor :: WithDiffuseTexture :: WithSpecular :: WithRefraction

compile = @(type, source)
	shader = gl.Shader(type
	shader.source(source
	shader.compile(
	shader.get_parameteri(gl.COMPILE_STATUS) == gl.FALSE && throw Throwable(source + shader.get_info_log()
	shader
$compile = compile

link = @(vshader, fshader)
	program = gl.Program(
	program.attach_shader(vshader
	program.attach_shader(fshader
	program.link(
	program.get_parameteri(gl.LINK_STATUS) == gl.FALSE && throw Throwable(program.get_info_log(
	program
$link = link

$__call = Class() :: @
	vertex_shader = @(defines)
		compile(gl.VERTEX_SHADER, defines + "
uniform mat4 projection;
uniform mat4 vertexMatrix;
attribute vec3 vertex;
#ifdef USE_TEXTURE
attribute vec2 texcoord;
varying vec2 varyingTexcoord;
#endif

void main()
{
	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));
#ifdef USE_TEXTURE
	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
#endif
}
"
	vertex_shader_normal = @(defines)
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
"
	constant_shader = @(defines)
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
"
	blinn_shader = @(defines)
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
	vec3 normal = normalize(varyingNormal);
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
	vec3 eye = vec3(0.0, 0.0, 1.0);
	vec3 h = normalize(light + eye);
	float nh = dot(normal, h);
	float c2 = shininess * shininess;
	float D = c2 / (nh * nh * (c2 - 1.0) + 1.0);
	float eh = dot(eye, h);
	float ne = dot(normal, eye);
	float nl = dot(normal, light);
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
	* max(nl, 0.0) + specular * max(D * D * G * F / ne, 0.0);
}
"
	lambert_shader = @(defines)
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
#ifdef USE_TEXTURE
	vec4 sample = texture2D(diffuse, varyingTexcoord);
	if (sample.a < 0.5) discard;
#endif
	vec3 normal = normalize(varyingNormal);
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
#ifdef USE_TEXTURE
	gl_FragColor = color + sample * max(dot(normal, light), 0.0);
#else
	gl_FragColor = color + diffuse * max(dot(normal, light), 0.0);
#endif
}
"
	phong_shader = @(defines)
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
	vec3 normal = normalize(varyingNormal);
	vec3 light = normalize(vec3(5.0, 5.0, 10.0));
	vec3 r = reflect(-light, normal);
	vec3 eye = vec3(0.0, 0.0, 1.0);
#ifdef USE_TEXTURE
	gl_FragColor = color + sample
#else
	gl_FragColor = color + diffuse
#endif
	* max(dot(normal, light), 0.0) + specular * pow(max(dot(r, eye), 0.0), shininess);
}
"
	skin_shader = @(joints, weights, defines)
		joint = ""
		weight = ""
		vertex = ""
		for i = 1; i < weights; i = i + 1
			joint = joint + "attribute float joint" + i + ";"
			weight = weight + "attribute float weight" + i + ";"
			vertex = vertex + "if (weight" + i + " > 0.0) { vm += vertexMatrices[int(joint" + i + ")] * weight" + i + ";"
		for i = 1; i < weights; i = i + 1
			vertex = vertex + " }"
		compile(gl.VERTEX_SHADER, defines + "
mat3 invert4to3(const in mat4 m) {
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
	mat4 vm = vertexMatrices[int(joint0)] * weight0;" + vertex + "
	gl_Position = projection * (vm * v);
	varyingNormal = normalize(normal * invert4to3(vm));
#ifdef USE_TEXTURE
	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
#endif
}
"
	lazy = @(f)
		g = @
			x = f(
			:g = @ x
			x
		@ g(
	lazy_xs = @(f)
		c = {
		@(*xs) c.has(xs) ? c[xs] : (c[xs] = f(*xs))
	skins_color = Object(
	skins_color.constant = '('SkinColorShader, 'constant_shader_color
	skins_color.blinn = '('SkinDiffuseColorSpecularRefractionShader, 'blinn_shader_color
	skins_color.lambert = '('SkinDiffuseColorShader, 'lambert_shader_color
	skins_color.phong = '('SkinDiffuseColorSpecularShader, 'phong_shader_color
	skins_texture = Object(
	skins_texture.constant = '('SkinTextureShader, 'constant_shader_texture
	skins_texture.blinn = '('SkinDiffuseTextureSpecularRefractionShader, 'blinn_shader_texture
	skins_texture.lambert = '('SkinDiffuseTextureShader, 'lambert_shader_texture
	skins_texture.phong = '('SkinDiffuseTextureSpecularShader, 'phong_shader_texture
	$__initialize = @
		$vertex_shader = lazy(@ vertex_shader(""
		$vertex_shader_texture = lazy(@ vertex_shader("#define USE_TEXTURE"
		$vertex_shader_normal = lazy(@ vertex_shader_normal(""
		$vertex_shader_normal_texture = lazy(@ vertex_shader_normal("#define USE_TEXTURE"
		$constant_shader_color = lazy(@ constant_shader(""
		$constant_shader_texture = lazy(@ constant_shader("#define USE_TEXTURE"
		$constant_color = lazy((@ ColorShader(link($vertex_shader(), $constant_shader_color())))[$]
		$constant_texture = lazy((@ TextureShader(link($vertex_shader_texture(), $constant_shader_texture())))[$]
		$blinn_shader_color = lazy(@ blinn_shader(""
		$blinn_shader_texture = lazy(@ blinn_shader("#define USE_TEXTURE"
		$blinn_color = lazy((@ DiffuseColorSpecularRefractionShader(link($vertex_shader_normal(), $blinn_shader_color())))[$]
		$blinn_texture = lazy((@ DiffuseTextureSpecularRefractionShader(link($vertex_shader_normal_texture(), $blinn_shader_texture())))[$]
		$lambert_shader_color = lazy(@ lambert_shader(""
		$lambert_shader_texture = lazy(@ lambert_shader("#define USE_TEXTURE"
		$lambert_color = lazy((@ DiffuseColorShader(link($vertex_shader_normal(), $lambert_shader_color())))[$]
		$lambert_texture = lazy((@ DiffuseTextureShader(link($vertex_shader_normal_texture(), $lambert_shader_texture())))[$]
		$phong_shader_color = lazy(@ phong_shader(""
		$phong_shader_texture = lazy(@ phong_shader("#define USE_TEXTURE"
		$phong_color = lazy((@ DiffuseColorSpecularShader(link($vertex_shader_normal(), $phong_shader_color())))[$]
		$phong_texture = lazy((@ DiffuseTextureSpecularShader(link($vertex_shader_normal_texture(), $phong_shader_texture())))[$]
		$skin_shader_normal = lazy_xs(@(joints, weights) skin_shader(joints, weights, ""
		$skin_shader_normal_texture = lazy_xs(@(joints, weights) skin_shader(joints, weights, "#define USE_TEXTURE"
		$SkinColorShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithColor
		$SkinDiffuseColorShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseColor
		$SkinDiffuseColorSpecularShader = @(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseColor :: WithSpecular
		$SkinDiffuseColorSpecularRefractionShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseColor :: WithSpecular :: WithRefraction
		$skin_color = lazy_xs((@(joints, weights, model)
			factory = skins_color.(model)
			$.(factory[0])(weights)(link($skin_shader_normal(joints, weights), $.(factory[1])()
		)[$]
		$SkinTextureShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithTexture
		$SkinDiffuseTextureShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseTexture
		$SkinDiffuseTextureSpecularShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseTexture :: WithSpecular
		$SkinDiffuseTextureSpecularRefractionShader = lazy_xs(@(n) Class(SkinShader(n)) :: WithSkinNormal :: WithColor :: WithDiffuseTexture :: WithSpecular :: WithRefraction
		$skin_texture = lazy_xs((@(joints, weights, model)
			factory = skins_texture.(model)
			$.(factory[0])(weights)(link($skin_shader_normal_texture(joints, weights), $.(factory[1])()
		)[$]
