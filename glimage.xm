gl = Module("gl");
glshaders = Module("glshaders");
cairo = Module("cairo");

get_rgba = @(image0) {
	width = image0.get_width();
	height = image0.get_height();
	image1 = cairo.ImageSurface(cairo.Format.ARGB32, width, height);
	try {
		context = cairo.Context(image1);
		try {
			context.set_operator(cairo.Operator.CLEAR);
			context.paint();
			context.set_operator(cairo.Operator.OVER);
			context.set_source(image0, 0.0, 0.0);
			context.paint();
		} finally {
			context.release();
		}
		data = image1.get_data();
		n = width * height;
		for (i = 0; i < n; i = i + 1) {
			t = data[i * 4];
			data[i * 4] = data[i * 4 + 2];
			data[i * 4 + 2] = t;
		}
		'(width, height, data);
	} finally {
		image1.release();
	}
};

load_rgba = @(path) {
	image = cairo.ImageSurface.create_from_file(path);
	try {
		get_rgba(image);
	} finally {
		image.release();
	}
};
$load_rgba = load_rgba;

$Image = Class() :: @{
	$create = @(width, height, data) {
		$width = Float(width);
		$height = Float(height);
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
"));
		$projection = $program.get_uniform_location("projection");
		$vertex_matrix = $program.get_uniform_location("vertexMatrix");
		$vertex = $program.get_attrib_location("vertex");
		$texcoord = $program.get_attrib_location("texcoord");
		$color = $program.get_uniform_location("color");
		$texture = gl.Texture();
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, $texture);
		gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.bind_texture(gl.TEXTURE_2D, null);
		$vertices = gl.Buffer();
	};
	$__initialize = @(path) {
		rgba = load_rgba(path);
		$create(rgba[0], rgba[1], rgba[2]);
	};
	$destroy = @{
		$program.delete();
		$texture.delete();
		$vertices.delete();
	};
	$__call = @(projection, vertex, bytes) {
		gl.use_program($program);
		$projection.matrix4fv(false, projection);
		$vertex_matrix.matrix4fv(false, vertex);
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices);
		gl.buffer_data(gl.ARRAY_BUFFER, bytes, gl.STREAM_DRAW);
		gl.enable_vertex_attrib_array($vertex);
		bpe = gl.Float32Array.BYTES_PER_ELEMENT;
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 5 * bpe, 0);
		gl.enable_vertex_attrib_array($texcoord);
		gl.vertex_attrib_pointer($texcoord, 2, gl.FLOAT, false, 5 * bpe, 3 * bpe);
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, $texture);
		$color.uniform1i(0);
		gl.draw_arrays(gl.TRIANGLE_STRIP, 0, bytes.size() / (5 * bpe));
		gl.use_program(null);
		gl.bind_buffer(gl.ARRAY_BUFFER, null);
		gl.bind_texture(gl.TEXTURE_2D, null);
	};
};

$Font = Class() :: @{
	$create = @(width, height, data, count, unit) {
		$unit = unit;
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
"));
		$projection = $program.get_uniform_location("projection");
		$vertex_matrix = $program.get_uniform_location("vertexMatrix");
		$offset = $program.get_uniform_location("offset");
		$vertex = $program.get_attrib_location("vertex");
		$color = $program.get_uniform_location("color");
		$texture = gl.Texture();
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, $texture);
		gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.bind_texture(gl.TEXTURE_2D, null);
		bytes = Bytes(count * 12 * gl.Float32Array.BYTES_PER_ELEMENT);
		array = gl.Float32Array(bytes);
		division = 1.0 / count;
		for (i = 0; i < count; i = i + 1) {
			j = i * 12;
			array[j] = array[j + 3] = array[j + 4] = array[j + 10] = 0.0;
			array[j + 6] = array[j + 9] = unit;
			array[j + 1] = array[j + 7] = 1.0;
			array[j + 2] = array[j + 5] = i * division;
			array[j + 8] = array[j + 11] = (i + 1) * division;
		}
		$vertices = gl.Buffer();
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices);
		gl.buffer_data(gl.ARRAY_BUFFER, bytes, gl.STATIC_DRAW);
		gl.bind_buffer(gl.ARRAY_BUFFER, null);
	};
	$__initialize = @{
		image = cairo.ImageSurface(cairo.Format.ARGB32, 32 * 128, 64);
		try {
			context = cairo.Context(image);
			try {
				context.select_font_face("Monospace", cairo.FontSlant.NORMAL, cairo.FontWeight.BOLD);
				context.set_font_size(48);
				for (i = 32; i < 127; i = i + 1) {
					context.move_to(i * 32 + 2, 64 - 3);
					context.text_path(String.from_code(i));
					context.set_source_rgb(0.0, 0.0, 0.0);
					context.set_line_width(4.0);
					context.stroke_preserve();
					context.set_source_rgb(1.0, 1.0, 1.0);
					context.fill();
				}
			} finally {
				context.release();
			}
			rgba = get_rgba(image);
			$create(rgba[0], rgba[1], rgba[2], 128, 0.5);
		} finally {
			image.release();
		}
	};
	$destroy = @{
		$program.delete();
		$texture.delete();
		$vertices.delete();
	};
	$__call = @(projection, vertex, text) {
		gl.use_program($program);
		$projection.matrix4fv(false, projection);
		$vertex_matrix.matrix4fv(false, vertex);
		gl.enable_vertex_attrib_array($vertex);
		gl.bind_buffer(gl.ARRAY_BUFFER, $vertices);
		gl.vertex_attrib_pointer($vertex, 3, gl.FLOAT, false, 0, 0);
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, $texture);
		$color.uniform1i(0);
		n = text.size();
		for (i = 0; i < n; i = i + 1) {
			$offset.uniform1f(i * $unit);
			gl.draw_arrays(gl.TRIANGLE_STRIP, text.code_at(i) * 4, 4);
		}
		gl.use_program(null);
		gl.bind_buffer(gl.ARRAY_BUFFER, null);
		gl.bind_texture(gl.TEXTURE_2D, null);
	};
};
