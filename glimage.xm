io = Module("io"
gl = Module("gl"
glshaders = Module("glshaders"
skia = Module("skia"

load_rgba = @(path)
	file = io.File(path, "r"
	try
		image = skia.Image.create_from_stream(file.read
		try
			width = image.width(
			height = image.height(
			surface = skia.Surface.raster(width, height
			try
				surface.draw(@(canvas) canvas.draw_image(image, 0.0, 0.0
				'(width, height, surface.read_pixels(width, height, skia.ColorType.RGBA_8888, skia.AlphaType.UNPREMUL, 0, 0
			finally
				surface.dispose(
		finally
			image.dispose(
	finally
		file.close(
$load_rgba = load_rgba

$Image = Object + @
	$width
	$height
	$program
	$projection
	$vertex_matrix
	$vertex
	$texcoord
	$color
	$texture
	$vertices
	$create = @(width, height, data)
		$width = Float(width
		$height = Float(height
		$program = glshaders.link(glshaders.compile(gl.VERTEX_SHADER, "
uniform mat4 projection;
uniform mat4 vertexMatrix;
attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 varyingTexcoord;

void main()
{
	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));
	varyingTexcoord = texcoord;
}
"), glshaders.compile(gl.FRAGMENT_SHADER, "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D color;
varying vec2 varyingTexcoord;

void main()
{
	vec4 sample = texture2D(color, varyingTexcoord);
	if (sample.a < 0.5) discard;
	gl_FragColor = sample;
}
")
		$projection = $program.get_uniform_location("projection"
		$vertex_matrix = $program.get_uniform_location("vertexMatrix"
		$vertex = $program.get_attrib_location("vertex"
		$texcoord = $program.get_attrib_location("texcoord"
		$color = $program.get_uniform_location("color"
		$texture = gl.Texture(
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, $texture
		gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR
		gl.bind_texture(gl.TEXTURE_2D, null
		$vertices = gl.Buffer(
	$__initialize = @(path) $create(*load_rgba(path
	$destroy = @
		$program.delete(
		$texture.delete(
		$vertices.delete(
	$__call = @(projection, vertex, bytes)
		gl.use_program($program
		$projection.matrix4fv(false, projection
		$vertex_matrix.matrix4fv(false, vertex
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices
		gl.buffer_data(gl.ARRAY_BUFFER, bytes, gl.STREAM_DRAW
		gl.enable_vertex_attrib_array($vertex
		bpe = gl.Float32Array.BYTES_PER_ELEMENT
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 5 * bpe, 0
		gl.enable_vertex_attrib_array($texcoord
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, 5 * bpe, 3 * bpe
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, $texture
		$color.uniform1i(0
		gl.draw_arrays(gl.TRIANGLE_STRIP, 0, bytes.size() / (5 * bpe)
		gl.use_program(null
		gl.bind_buffer(gl.ARRAY_BUFFER, null
		gl.bind_texture(gl.TEXTURE_2D, null

$Font = Object + @
	$unit
	$program
	$projection
	$vertex_matrix
	$offset
	$vertex
	$color
	$texture
	$vertices
	$create = @(width, height, data, count, unit)
		$unit = unit
		$program = glshaders.link(glshaders.compile(gl.VERTEX_SHADER, "
uniform mat4 projection;
uniform mat4 vertexMatrix;
uniform float offset;
attribute vec3 vertex;
varying vec2 varyingTexcoord;

void main()
{
	gl_Position = projection * (vertexMatrix * vec4(offset + vertex.x, vertex.y, 0.0, 1.0));
	varyingTexcoord = vec2(vertex.z, 1.0 - vertex.y);
}
"), glshaders.compile(gl.FRAGMENT_SHADER, "
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D color;
varying vec2 varyingTexcoord;

void main()
{
	vec4 sample = texture2D(color, varyingTexcoord);
	if (sample.a < 0.5) discard;
	gl_FragColor = sample;
}
")
		$projection = $program.get_uniform_location("projection"
		$vertex_matrix = $program.get_uniform_location("vertexMatrix"
		$offset = $program.get_uniform_location("offset"
		$vertex = $program.get_attrib_location("vertex"
		$color = $program.get_uniform_location("color"
		$texture = gl.Texture(
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, $texture
		gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR
		gl.bind_texture(gl.TEXTURE_2D, null
		bytes = Bytes(count * 12 * gl.Float32Array.BYTES_PER_ELEMENT
		array = gl.Float32Array(bytes
		division = 1.0 / count
		for i = 0; i < count; i = i + 1
			j = i * 12
			array[j] = array[j + 3] = array[j + 4] = array[j + 10] = 0.0
			array[j + 6] = array[j + 9] = unit
			array[j + 1] = array[j + 7] = 1.0
			array[j + 2] = array[j + 5] = i * division
			array[j + 8] = array[j + 11] = (i + 1) * division
		$vertices = gl.Buffer(
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices
		gl.buffer_data(gl.ARRAY_BUFFER, bytes, gl.STATIC_DRAW
		gl.bind_buffer(gl.ARRAY_BUFFER, null
	$__initialize = @(typeface)
		font = skia.Font(typeface, 48.0
		width = 32 * 128
		height = 64
		surface = skia.Surface.raster(width, height
		stroke = skia.Paint(
		stroke.color__(0xff000000
		fill = skia.Paint(
		fill.color__(0xffffffff
		try
			surface.draw(@(canvas)
				canvas.clear(0x00000000
				for i = 32; i < 127; i = i + 1
					s = String.from_code(i
					x = i * 32 + 2
					y = 64 - 3
					for j = -2; j <= 2; j = j + 1
						for k = -2; k <= 2; k = k + 1
							canvas.draw_text(s, x + k, y + j, font, stroke
					canvas.draw_text(s, x, y, font, fill
			$create(width, height
				surface.read_pixels(width, height, skia.ColorType.RGBA_8888, skia.AlphaType.UNPREMUL, 0, 0
				128, 0.5
		finally
			stroke.dispose(
			fill.dispose(
			surface.dispose(
			font.dispose(
	$destroy = @
		$program.delete(
		$texture.delete(
		$vertices.delete(
	$__call = @(projection, vertex, text)
		gl.use_program($program
		$projection.matrix4fv(false, projection
		$vertex_matrix.matrix4fv(false, vertex
		gl.enable_vertex_attrib_array($vertex
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 0, 0
		gl.active_texture(gl.TEXTURE0
		gl.bind_texture(gl.TEXTURE_2D, $texture
		$color.uniform1i(0
		n = text.size(
		for i = 0; i < n; i = i + 1
			$offset.uniform1f(i * $unit
			gl.draw_arrays(gl.TRIANGLE_STRIP, text.code_at(i) * 4, 4
		gl.use_program(null
		gl.bind_buffer(gl.ARRAY_BUFFER, null
		gl.bind_texture(gl.TEXTURE_2D, null
