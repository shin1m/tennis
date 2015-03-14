system = Module("system");
print = system.out.write_line;
io = Module("io");
math = Module("math");
time = Module("time");
libxml = Module("libxml");
gl = Module("gl");
glmatrix = Module("glmatrix");
glshaders = Module("glshaders");
glimage = Module("glimage");

print_time = false;

Matrix3 = glmatrix.Matrix3;
Matrix4 = glmatrix.Matrix4;
Vector3 = glmatrix.Vector3;
Vector4 = glmatrix.Vector4;

find_index = @(text, i, predicate) {
	n = text.size();
	for (; i < n; i = i + 1) if (predicate(text.code_at(i))) break;
	i;
};
parse_texts = @(text, delimiter, callback) {
	i = 0;
	while (true) {
		i = find_index(text, i, @(x) !delimiter(x));
		if (i >= text.size()) break;
		j = find_index(text, i, delimiter);
		callback(text.substring(i, j - i));
		i = j;
	}
};
is_whitespace = @(x) x <= 0x20;
parse_array = @(type, parse, count, text) {
	array = type(Bytes(count * type.BYTES_PER_ELEMENT));
	i = 0;
	parse_texts(text, is_whitespace, @(x) {
		array[i] = parse(x);
		:i = i + 1;
	});
	array;
};
$parse_array = parse_array;

Reader = Class(libxml.TextReader) :: @{
	$__initialize = @(source) {
		:$^__initialize[$](source);
		$_type = null;
	};
	$read_next = @() $_type = $read() ? $node_type() : null;
	$type = @() $_type;
	$move_to_tag = @{
		while ($_type !== null && $_type != libxml.ReaderTypes.ELEMENT && $_type != libxml.ReaderTypes.END_ELEMENT) $read_next();
	};
	$is_start_element = @(name) {
		$move_to_tag();
		$_type == libxml.ReaderTypes.ELEMENT && $local_name() == name;
	};
	$check_start_element = @(name) {
		if (!$is_start_element(name)) throw Throwable("must be element: " + name);
	};
	$start_element = @(name) {
		$check_start_element(name);
		b = $is_empty_element();
		$read_next();
		!b;
	};
	$end_element = @{
		$move_to_tag();
		if ($_type != libxml.ReaderTypes.END_ELEMENT) throw Throwable("must be end of element.");
		$read_next();
	};
	$read_element_text = @{
		if ($is_empty_element()) {
			$read_next();
			return "";
		}
		text = "";
		$read_next();
		while ($_type != libxml.ReaderTypes.END_ELEMENT) {
			if ($_type == libxml.ReaderTypes.TEXT || $_type == libxml.ReaderTypes.CDATA) {
				text = text + $value();
				$read_next();
			} else if ($_type == libxml.ReaderTypes.ELEMENT) {
				text = text + $read_element_text();
			} else {
				$read_next();
			}
		}
		$read_next();
		text;
	};
	$read_element = @(name) {
		$check_start_element(name);
		$read_element_text();
	};
	$parse_elements = @(elements, x) {
		$read_next();
		while (true) {
			$move_to_tag();
			if ($type() != libxml.ReaderTypes.ELEMENT) break;
			try {
				element = elements[$local_name()];
			} catch (Throwable e) {
				$read_element_text();
				continue;
			}
			element(x);
		}
		$read_next();
	};
};
$Reader = Reader;

WithTree = @{
	$share = @() $tree('share);
	$own = @() $tree('own);
};

Input = Class() :: @{
	$__string = @() "[" + $offset + "] " + $source;
};

Source = Class() :: WithTree :: @{
	$__initialize = @() $built = false;
	$__string = @() "Source(" + $id + ")";
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$params.(symbol)();
	};
	$build = @(resolve) {
		if ($built) return;
		$_source = resolve($source);
		$built = true;
	};
	$__get_at = @(i) {
		j = $offset + i * $stride;
		source = $_source;
		x = [];
		$params.each(@(k) x.push(source[j + k]));
		x;
	};
	$get_matrix_at = @(i) '($matrix(i));
	$matrix = @(i) {
		j = $offset + i * $stride;
		xs = $_source;
		m = Matrix4();
		for (k = 0; k < 16; k = k + 1) m.v[k] = xs[j + k];
		m;
	};
	$create_array = @(n) {
		type = $_source.:;
		bytes = Bytes(n * type.BYTES_PER_ELEMENT);
		x = type(bytes);
		x.bytes = bytes;
		x;
	};
};

create_buffer = @(bytes) {
	buffer = gl.Buffer();
	gl.bind_buffer(gl.ARRAY_BUFFER, buffer);
	gl.buffer_data(gl.ARRAY_BUFFER, bytes, gl.STATIC_DRAW);
	buffer;
};

input_buffer = @(resolve, input, source, dimension) {
	source.build(resolve);
	n = $count * $unit;
	array = source.create_array(n * dimension);
	i = input.offset;
	for (j = 0; j < n; j = j + 1) {
		x = source[$indices[i]];
		for (k = 0; k < dimension; k = k + 1) array[j * dimension + k] = x[k];
		i = i + $stride;
	}
	create_buffer(array.bytes);
};

normal_and_others = @(resolve) {
	try {
		normals = $inputs["NORMAL"];
		source = resolve(normals.source);
		$_attributes.normals = input_buffer[$](resolve, normals, source, source.params.size());
	} catch (Throwable e) {
		$_attributes.normals = null;
	}
	$_others = [];
	$inputs.each(@(key, value) {
		if (key == "VERTEX" || key == "NORMAL") return;
		source = resolve(value.source);
		$_others.push('(key, input_buffer[$](resolve, value, source, value.semantic == "TEXCOORD" ? 2 : source.params.size())));
	}[$]);
	gl.bind_buffer(gl.ARRAY_BUFFER, null);
};

invert4to3 = @(m) {
	#d = m[0] * m[5] * m[10] + m[4] * m[9] * m[2] + m[8] * m[1] * m[6] - m[0] * m[9] * m[6] - m[8] * m[5] * m[2] - m[4] * m[1] * m[10];
	bytes = Bytes(9 * gl.Float32Array.BYTES_PER_ELEMENT);
	v = gl.Float32Array(bytes);
	v[0] = m[5] * m[10] - m[6] * m[9];
	v[1] = m[2] * m[9] - m[1] * m[10];
	v[2] = m[1] * m[6] - m[2] * m[5];
	v[3] = m[6] * m[8] - m[4] * m[10];
	v[4] = m[0] * m[10] - m[2] * m[8];
	v[5] = m[2] * m[4] - m[0] * m[6];
	v[6] = m[4] * m[9] - m[5] * m[8];
	v[7] = m[1] * m[8] - m[0] * m[9];
	v[8] = m[0] * m[5] - m[1] * m[4];
	bytes;
};

Primitive = Class() :: WithTree :: @{
	$__initialize = @{
		$inputs = {};
	};
	$__string = @() " " + {
		"count": $count,
		"material": $material,
		"stride": $stride,
		"inputs": $inputs
	} + " " + $indices.size() + " indices";
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$inputs.(symbol)();
		$inputs.each(@(key, value) value.(symbol)());
		$indices.(symbol)();
	};
	$destroy = @{
		$_attributes.vertices.delete();
		if ($_attributes.normals !== null) $_attributes.normals.delete();
		$_others.each(@(x) x[1].delete());
	};
	$build = @(resolve) {
		vertices = $inputs["VERTEX"];
		source = resolve(resolve(vertices.source).inputs["POSITION"]);
		$_attributes = glshaders.Attributes();
		$_attributes.vertices = input_buffer[$](resolve, vertices, source, source.params.size());
		normal_and_others[$](resolve);
	};
	$create = @(count, material, vertices, normals, others) {
		$count = count;
		$material = material;
		$_attributes = glshaders.Attributes();
		$_attributes.vertices = create_buffer(vertices);
		$_attributes.normals = normals === null ? null : create_buffer(normals);
		$_others = {};
		others.each((@(key, value) $_others.push('(key, create_buffer(value))))[$]);
		gl.bind_buffer(gl.ARRAY_BUFFER, null);
		$;
	};
};

Lines = Class(Primitive) :: @{
	$mode = gl.LINES;
	$unit = 2;
	$__string = @() "Lines" + :$^__string[$]();
};

Triangles = Class(Primitive) :: @{
	$mode = gl.TRIANGLES;
	$unit = 3;
	$__string = @() "Triangles" + :$^__string[$]();
};
$Triangles = Triangles;

Mesh = Class() :: WithTree :: @{
	$__initialize = @{
		$sources = [];
		$primitives = [];
		$built = false;
	};
	$__string = @() "Mesh(" + $id + ") " + $primitives;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$sources.(symbol)();
		$sources.each(@(x) x.(symbol)());
		$vertices.(symbol)();
		$vertices.inputs.(symbol)();
		$primitives.(symbol)();
		$primitives.each(@(x) x.(symbol)());
	};
	$destroy = @{
		if ($built) $primitives.each(@(x) x.destroy());
	};
	$build = @(resolve) {
		if ($built) return;
		$primitives.each(@(x) x.build(resolve));
		$built = true;
	};
};
$Mesh = Mesh;

Surface = Class() :: @{
	$__string = @() "Surface(" + $sid + ") {type: " + $type + ", from: " + $from + "}";
	$destroy = @{};
	$build = @(resolve, sids) {
		$_from = resolve("#" + $from);
		$_from.build(resolve);
	};
};

Sampler2D = Class() :: @{
	$__initialize = @{
		$minfilter = "NEAREST";
		$magfilter = "NEAREST";
		$built = false;
	};
	$__string = @() "Sampler2D(" + $sid + ") {source: " + $source + "}";
	$destroy = @{
		if ($built) $_texture.delete();
	};
	$build = @(resolve, sids) {
		if ($built) return;
		$_source = sids[$source];
		$_source.build(resolve, sids);
		from = $_source._from;
		$_texture = gl.Texture();
		gl.active_texture(gl.TEXTURE0);
		gl.bind_texture(gl.TEXTURE_2D, $_texture);
		gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, from.width, from.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, from.data);
		filter = gl.(Symbol($minfilter));
		if (filter === gl.NEAREST_MIPMAP_NEAREST || filter === gl.NEAREST_MIPMAP_LINEAR || filter === gl.LINEAR_MIPMAP_NEAREST || filter === gl.LINEAR_MIPMAP_LINEAR) gl.generate_mipmap(gl.TEXTURE_2D);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, filter);
		gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.(Symbol($magfilter)));
		gl.bind_texture(gl.TEXTURE_2D, null);
		$built = true;
	};
};

CommonColor = Class(Vector4) :: @{
	$__string = @() "CommonColor(" + $x + ", " + $y + ", " + $z + ", " + $w + ")";
	$build = @(resolve, sids) {};
};

CommonParam = Class() :: @{
	$__string = @() "CommonParam(" + $ref + ")";
	$build = @(resolve, sids) $_ref = sids[$ref];
};

Texture = Class() :: @{
	$__string = @() "Texture(" + $texture + ") {texcoord: " + $texcoord + "}";
	$build = @(resolve, sids) {
		$_texture = sids[$texture];
		$_texture.build(resolve, sids);
	};
};

CommonFloat = Class() :: @{
	$__initialize = @(value) $value = value;
	$__string = @() "CommonFloat(" + $value + ")";
	$build = @(resolve, sids) {};
};

ShadingModel = Class() :: WithTree :: @{
	$__initialize = @{
		$emission = CommonColor(0.0, 0.0, 0.0, 1.0);
		$ambient = CommonColor(0.0, 0.0, 0.0, 1.0);
		$diffuse = CommonColor(0.0, 0.0, 0.0, 1.0);
		$specular = CommonColor(0.0, 0.0, 0.0, 1.0);
		$shininess = CommonFloat(0.0);
		$reflective = CommonColor(0.0, 0.0, 0.0, 1.0);
		$reflectivity = CommonFloat(0.0);
		$transparent = CommonColor(0.0, 0.0, 0.0, 1.0);
		$transparency = CommonFloat(0.0);
		$index_of_refraction = CommonFloat(0.0);
	};
	$__string = @{
		"emission: " + $emission +
		", ambient: " + $ambient +
		", diffuse: " + $diffuse +
		", specular: " + $specular +
		", shininess: " + $shininess +
		", reflective: " + $reflective +
		", reflectivity: " + $reflectivity +
		", transparent: " + $transparent +
		", transparency: " + $transparency +
		", index_of_refraction: " + $index_of_refraction;
	};
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$emission.(symbol)();
		$ambient.(symbol)();
		$diffuse.(symbol)();
		$specular.(symbol)();
		$shininess.(symbol)();
		$reflective.(symbol)();
		$reflectivity.(symbol)();
		$transparent.(symbol)();
		$transparency.(symbol)();
		$index_of_refraction.(symbol)();
	};
	$build = @(resolve, sids) {
		$emission.build(resolve, sids);
		$ambient.build(resolve, sids);
		$diffuse.build(resolve, sids);
		$specular.build(resolve, sids);
		$shininess.build(resolve, sids);
		$reflective.build(resolve, sids);
		$reflectivity.build(resolve, sids);
		$transparent.build(resolve, sids);
		$transparency.build(resolve, sids);
		$index_of_refraction.build(resolve, sids);
	};
};

Blinn = Class(ShadingModel) :: @{
	$__string = @() "Blinn {" + :$^__string[$]() + "}";
	$mesh_shader = @(shaders) $diffuse.: === Texture ? shaders.blinn_texture() : shaders.blinn_color();
	$skin_shader = @(shaders, joints, weights) $diffuse.: === Texture ? shaders.skin_texture(joints, weights, 'blinn) : shaders.skin_color(joints, weights, 'blinn);
	$setup = @(uniforms, attributes, others) {
		uniforms.color = $emission + $ambient * 0.125;
		uniforms.diffuse = $diffuse.: === Texture ? $diffuse._texture._texture : $diffuse;
		uniforms.specular = $specular;
		uniforms.shininess = $shininess.value;
		uniforms.refraction = $index_of_refraction.value;
		if ($diffuse.: === Texture) attributes.texcoords = others[$diffuse.texcoord];
	};
};

Constant = Class(ShadingModel) :: @{
	$__string = @() "Constant {" + :$^__string[$]() + "}";
	$mesh_shader = @(shaders) shaders.constant_color();
	$setup = @(uniforms, attributes, others) {
		uniforms.color = $emission + $ambient * 0.125;
	};
	$__call = @(vertex, normal, vertices, normals, others, mode, offset, count) {
		uniforms = glshaders.Uniforms();
		uniforms.projection = projection;
		uniforms.vertex = vertex;
		uniforms.color = $emission + $ambient * 0.125;
		attributes = glshaders.Attributes();
		attributes.vertices = vertices;
		$_shader(uniforms, attributes, mode, offset, count);
	};
};

Lambert = Class(ShadingModel) :: @{
	$__string = @() "Lambert {" + :$^__string[$]() + "}";
	$mesh_shader = @(shaders) $diffuse.: === Texture ? shaders.lambert_texture() : shaders.lambert_color();
	$skin_shader = @(shaders, joints, weights) $diffuse.: === Texture ? shaders.skin_texture(joints, weights, 'lambert) : shaders.skin_color(joints, weights, 'lambert);
	$setup = @(uniforms, attributes, others) {
		uniforms.color = $emission + $ambient * 0.125;
		uniforms.diffuse = $diffuse.: === Texture ? $diffuse._texture._texture : $diffuse;
		if ($diffuse.: === Texture) attributes.texcoords = others[$diffuse.texcoord];
	};
};

Phong = Class(ShadingModel) :: @{
	$__string = @() "Phong {" + :$^__string[$]() + "}";
	$mesh_shader = @(shaders) $diffuse.: === Texture ? shaders.phong_texture() : shaders.phong_color();
	$skin_shader = @(shaders, joints, weights) $diffuse.: === Texture ? shaders.skin_texture(joints, weights, 'phong) : shaders.skin_color(joints, weights, 'phong);
	$setup = @(uniforms, attributes, others) {
		uniforms.color = $emission + $ambient * 0.125;
		uniforms.diffuse = $diffuse.: === Texture ? $diffuse._texture._texture : $diffuse;
		uniforms.specular = $specular;
		uniforms.shininess = $shininess.value;
		if ($diffuse.: === Texture) attributes.texcoords = others[$diffuse.texcoord];
	};
};

TechniqueFX = Class() :: WithTree :: @{
	$__string = @() "TechniqueFX(" + $sid + ") " + $model;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$model.(symbol)();
	};
	$build = @(resolve, sids) $model.build(resolve, sids);
};

ProfileCOMMON = Class() :: WithTree :: @{
	$__initialize = @{
		$sids = {};
		$newparams = [];
	};
	$__string = @() "ProfileCOMMON " + $newparams + " " + $technique;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$sids.(symbol)();
		$newparams.(symbol)();
		$newparams.each(@(x) x.(symbol)());
		$technique.(symbol)();
	};
	$destroy = @() $newparams.each(@(x) x.destroy());
	$build = @(resolve) {
		$newparams.each((@(x) x.build(resolve, $sids))[$]);
		$technique.build(resolve, $sids);
	};
};

Effect = Class() :: WithTree :: @{
	$__initialize = @{
		$profiles = [];
		$built = false;
	};
	$__string = @() "Effect(" + $id + ") " + $profiles;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$profiles.(symbol)();
		$profiles.each(@(x) x.(symbol)());
	};
	$destroy = @{
		if ($built) $profiles.each(@(x) x.destroy());
	};
	$build = @(resolve) {
		if ($built) return;
		$profiles.each(@(x) x.build(resolve));
		$built = true;
	};
};

Image = Class() :: @{
	$__initialize = @() $built = false;
	$__string = @() "Image {type: " + $type + ", value: " + $value + "}";
	$build = @(resolve) {
		if ($built) return;
		if ($type == "data") {
			throw Throwable("not implemented");
		} else {
			rgba = glimage.load_rgba($value);
			$width = rgba[0];
			$height = rgba[1];
			$data = rgba[2];
		}
		$built = true;
	};
};

Material = Class() :: @{
	$__initialize = @() $built = false;
	$__string = @() "Material {instance_effect: " + $instance_effect + "}";
	$build = @(resolve) {
		if ($built) return;
		$_instance_effect = resolve($instance_effect);
		$_instance_effect.build(resolve);
		$built = true;
	};
	$model = @() $_instance_effect.profiles[0].technique.model;
};

Matrix = Class(Matrix4) :: @{
	$__string = @() "Matrix(" + :$^__string[$]() + ")";
	#$__call = @(x) x.multiply($);
	$__call = @(x) {
		v0 = $v;
		v1 = x.v;
		m0 = v1[0];
		m1 = v1[1];
		m2 = v1[2];
		m4 = v1[4];
		m5 = v1[5];
		m6 = v1[6];
		m8 = v1[8];
		m9 = v1[9];
		m10 = v1[10];
		v1[0] = m0 * v0[0] + m1 * v0[4] + m2 * v0[8];
		v1[1] = m0 * v0[1] + m1 * v0[5] + m2 * v0[9];
		v1[2] = m0 * v0[2] + m1 * v0[6] + m2 * v0[10];
		v1[3] = m0 * v0[3] + m1 * v0[7] + m2 * v0[11] + v1[3];
		v1[4] = m4 * v0[0] + m5 * v0[4] + m6 * v0[8];
		v1[5] = m4 * v0[1] + m5 * v0[5] + m6 * v0[9];
		v1[6] = m4 * v0[2] + m5 * v0[6] + m6 * v0[10];
		v1[7] = m4 * v0[3] + m5 * v0[7] + m6 * v0[11] + v1[7];
		v1[8] = m8 * v0[0] + m9 * v0[4] + m10 * v0[8];
		v1[9] = m8 * v0[1] + m9 * v0[5] + m10 * v0[9];
		v1[10] = m8 * v0[2] + m9 * v0[6] + m10 * v0[10];
		v1[11] = m8 * v0[3] + m9 * v0[7] + m10 * v0[11] + v1[11];
	};
};
$Matrix = Matrix;

Translate = Class(Vector3) :: @{
	$__string = @() "Translate(" + $x + ", " + $y + ", " + $z + ")";
	$__call = @(x) x.translate($x, $y, $z);
};
$Translate = Translate;

Rotate = Class(Vector4) :: @{
	$__string = @() "Rotate(" + $x + ", " + $y + ", " + $z + ", " + $w + ")";
	$__call = @(x) x.rotate(Vector3($x, $y, $z), $w);
};
$Rotate = Rotate;

Scale = Class(Vector3) :: @{
	$__string = @() "Scale(" + $x + ", " + $y + ", " + $z + ")";
	$__call = @(x) x.scale($x, $y, $z);
};
$Scale = Scale;

InstanceMaterial = Class() :: WithTree :: @{
	$__initialize = @{
		$bind_vertex_inputs = {};
	};
	$__string = @() "InstanceMaterial(" + $symbol + ") " + $target;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$bind_vertex_inputs.(symbol)();
	};
	$build = @(resolve) {
		$_target = resolve($target);
		$_target.build(resolve);
	};
	$model = @() $_target.model();
};

InstanceMaterialFallback = Class() :: WithTree :: @{
	$__initialize = @{
		$bind_vertex_inputs = {};
	};
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$bind_vertex_inputs.(symbol)();
	};
	$build = @(resolve) {};
	$model = @() $;
	$mesh_shader = @(shaders) shaders.constant_color();
	$skin_shader = @(shaders, joints, weights) shaders.skin_color(joints, weights, 'constant);
	$setup = @(uniforms, attributes, others) uniforms.color = Vector4(1.0, 1.0, 1.0, 1.0);
};

InstanceGeometry = Class() :: WithTree :: @{
	$__initialize = @(fallback) {
		$materials = {"": fallback};
	};
	$__string = @() "InstanceGeometry(" + $url + ") " + $materials;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$materials.(symbol)();
		$materials.each(@(key, value) {
			if (key != "") value.(symbol)();
		});
	};
	$build_render = @{
		$_primitives = [];
		$_geometry.primitives.each(@(x) {
			shader = $_shaders[x.material];
			others = {};
			x._others.each(@(x) {
				key = x[0];
				if (shader.bind_vertex_inputs.has(key)) key = shader.bind_vertex_inputs[key];
				others[key] = x[1];
			});
			uniforms = glshaders.Uniforms();
			$_primitives.push(@(projection, viewing) {
				uniforms.projection = projection;
				uniforms.vertex = viewing.bytes;
				uniforms.normal = invert4to3(viewing);
				shader.model.setup(uniforms, $_attributes, others);
				shader.shader(uniforms, $_attributes, $mode, 0, $count * $unit);
			}[x]);
		}[$]);
	};
	$build = @(resolve, shaders) {
		$_geometry = resolve($url);
		$_geometry.build(resolve);
		$_shaders = {};
		$materials.each(@(key, value) {
			value.build(resolve);
			shader = Object();
			shader.model = value.model();
			shader.shader = shader.model.mesh_shader(shaders);
			shader.bind_vertex_inputs = value.bind_vertex_inputs;
			$_shaders[key] = shader;
		}[$]);
		$build_render();
	};
	$create = @(geometry, shaders) {
		$_geometry = geometry;
		$_shaders = shaders;
		$build_render();
		$;
	};
	$render = @(projection, viewing) {
		primitives = $_primitives;
		n = primitives.size();
		for (i = 0; i < n; i = i + 1) primitives[i](projection, viewing);
	};
};
$InstanceGeometry = InstanceGeometry;

Node = Class() :: WithTree :: @{
	$__initialize = @{
		$sids = {};
		$transforms = [];
		$controllers = [];
		$geometries = [];
		$instance_nodes = [];
		$nodes = [];
		$built = false;
	};
	$__string = @() "Node(" + $id + ") " + $transforms + " " + $controllers + " " + $geometries + " " + $instance_nodes + " " + $nodes;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$sids.(symbol)();
		$transforms.(symbol)();
		$transforms.each(@(x) x.(symbol)());
		$controllers.(symbol)();
		$controllers.each(@(x) x.(symbol)());
		$geometries.(symbol)();
		$geometries.each(@(x) x.(symbol)());
		$instance_nodes.(symbol)();
		$instance_nodes.each(@(x) x.(symbol)());
		$nodes.(symbol)();
		$nodes.each(@(x) x.(symbol)());
	};
	$build = @(resolve, shaders) {
		if ($built) return;
		$controllers.each(@(x) x.build(resolve, shaders));
		$geometries.each(@(x) x.build(resolve, shaders));
		$_nodes = [];
		$instance_nodes.each(@(x) {
			node = resolve(x);
			node.build(resolve, shaders);
			$_nodes.push(node);
		}[$]);
		$nodes.each(@(x) x.build(resolve, shaders));
		$_controllers = [];
		$built = true;
	};
	$build_render = @{
		transforms = $transforms;
		controllers = $controllers;
		cn = controllers.size();
		nodes0 = $_nodes;
		nodes1 = $nodes;
		if ($joint) {
			roots = $_root_controllers;
			rn = roots.size();
			sid = $sid;
			$render = @(projection, viewing, joints) {
				tn = transforms.size();
				if (tn > 0) {
					viewing = Matrix4(viewing);
					for (i = 0; i < tn; i = i + 1) transforms[i](viewing);
				}
				for (i = 0; i < cn; i = i + 1) controllers[i]._joints0.vertex = viewing;
				for (i = 0; i < rn; i = i + 1) joints.push(roots[i]._joint2matrix);
				if (sid != "") {
					jn = joints.size();
					for (i = 0; i < jn; i = i + 1) {
						x = joints[i];
						if (x.has(sid)) x[sid].vertex = viewing;
					}
				}
				n0n = nodes0.size();
				for (i = 0; i < n0n; i = i + 1) nodes0[i].render(projection, viewing, joints);
				n1n = nodes1.size();
				for (i = 0; i < n1n; i = i + 1) nodes1[i].render(projection, viewing, joints);
				for (i = 0; i < rn; i = i + 1) joints.pop();
			};
		} else {
			geometries = $geometries;
			$render = @(projection, viewing, joints) {
				tn = transforms.size();
				if (tn > 0) {
					viewing = Matrix4(viewing);
					for (i = 0; i < tn; i = i + 1) transforms[i](viewing);
				}
				for (i = 0; i < cn; i = i + 1) controllers[i]._joints0.vertex = viewing;
				gn = geometries.size();
				for (i = 0; i < gn; i = i + 1) geometries[i].render(projection, viewing);
				n0n = nodes0.size();
				for (i = 0; i < n0n; i = i + 1) nodes0[i].render(projection, viewing, joints);
				n1n = nodes1.size();
				for (i = 0; i < n1n; i = i + 1) nodes1[i].render(projection, viewing, joints);
			};
		}
	};
	$postbuild = @(ancestors, controllers) {
		$controllers.each(controllers.push);
		if ($joint) {
			$_root_controllers = [];
			$_controllers.each(@(x) {
				if (ancestors.has(x)) return;
				ancestors[x] = null;
				$_root_controllers.push(x);
			}[$]);
		}
		$_nodes.each(@(x) x.postbuild(ancestors, controllers));
		$nodes.each(@(x) x.postbuild(ancestors, controllers));
		if ($joint) $_root_controllers.each(ancestors.remove);
		$build_render();
	};
	$create = @() {
		$joint = false;
		$_nodes = [];
		$_controllers = [];
		$build_render();
		$;
	};
};
$Node = Node;

SkinPrimitive = Class() :: WithTree :: @{
	$__initialize = @(primitive) {
		$mode = primitive.mode;
		$unit = primitive.unit;
		$count = primitive.count;
		$material = primitive.material;
		$stride = primitive.stride;
		$inputs = primitive.inputs;
		$indices = primitive.indices;
	};
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$inputs.(symbol)();
		$inputs.each(@(key, value) value.(symbol)());
		$indices.(symbol)();
	};
	$destroy = @{
		$_attributes.vertices.delete();
		$_attributes.joints.delete();
		$_attributes.weights.delete();
		if ($_attributes.normals !== null) $_attributes.normals.delete();
		$_others.each(@(x) x[1].delete());
	};
	$vertex_buffer = @(input, vertices, weights) {
		n = $count * $unit;
		bytes = Bytes(n * 3 * gl.Float32Array.BYTES_PER_ELEMENT);
		array = gl.Float32Array(bytes);
		jbytes = Bytes(n * weights * gl.Float32Array.BYTES_PER_ELEMENT);
		jarray = gl.Float32Array(jbytes);
		wbytes = Bytes(n * weights * gl.Float32Array.BYTES_PER_ELEMENT);
		warray = gl.Float32Array(wbytes);
		i = input.offset;
		for (j = 0; j < n; j = j + 1) {
			x = vertices[$indices[i]];
			vertex = x[0];
			bones = x[1];
			array[j * 3] = vertex.x;
			array[j * 3 + 1] = vertex.y;
			array[j * 3 + 2] = vertex.z;
			for (k = 0; k < bones.size(); k = k + 1) {
				jarray[j * weights + k] = bones[k][0] + 1.0;
				warray[j * weights + k] = bones[k][1];
			}
			if (k < 1) {
				jarray[j * weights + k] = 0.0;
				warray[j * weights + k] = 1.0;
				k = 1;
			}
			for (; k < weights; k = k + 1) {
				jarray[j * weights + k] = 0.0;
				warray[j * weights + k] = 0.0;
			}
			i = i + $stride;
		}
		$_attributes.vertices = create_buffer(bytes);
		$_attributes.joints = create_buffer(jbytes);
		$_attributes.weights = create_buffer(wbytes);
	};
	$build = @(resolve, vertices, weights) {
		$_attributes = glshaders.Attributes();
		$vertex_buffer($inputs["VERTEX"], vertices, weights);
		normal_and_others[$](resolve);
	};
};

Skin = Class() :: WithTree :: @{
	$__initialize = @{
		$bind_shape_matrix = Matrix4();
		$sources = [];
		$joints = {};
		$vertex_weights = {};
		$built = false;
	};
	$__string = @() "Skin(" + $id + ") {source: " + $source + ", bind_shape_matrix: " + $bind_shape_matrix + "}";
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$bind_shape_matrix.(symbol)();
		$sources.(symbol)();
		$sources.each(@(x) x.(symbol)());
		$joints.(symbol)();
		$vertex_weights.(symbol)();
		$vertex_weights.v.(symbol)();
	};
	$destroy = @{
		if ($built) $_primitives.each(@(x) x.destroy());
	};
	$build = @(resolve) {
		if ($built) return;
		$_source = resolve($source);
		position = resolve($_source.vertices.inputs["POSITION"]);
		position.build(resolve);
		ii = 0;
		ij = $vertex_weights["JOINT"][0];
		iw = $vertex_weights["WEIGHT"][0];
		weights = resolve($vertex_weights["WEIGHT"][1]);
		weights.build(resolve);
		vertices = [];
		$_weights_per_vertex = 0;
		n = $vertex_weights.count;
		for (i = 0; i < n; i = i + 1) {
			bones = [];
			m = $vertex_weights.vcount[i];
			if (m > $_weights_per_vertex) $_weights_per_vertex = m;
			for (j = 0; j < m; j = j + 1) {
				joint = $vertex_weights.v[ii + ij];
				weight = $vertex_weights.v[ii + iw];
				bones.push('(joint, weights[weight][0]));
				ii = ii + $vertex_weights.size();
			}
			x = position[i];
			vertices.push('(Vector3(x[0], x[1], x[2]), bones));
		}
		$_primitives = [];
		$_source.primitives.each(@(x) {
			y = SkinPrimitive(x);
			y.build(resolve, vertices, $_weights_per_vertex);
			$_primitives.push(y);
		}[$]);
		$built = true;
	};
};

InstanceController = Class() :: WithTree :: @{
	$__initialize = @(fallback) {
		$skeletons = [];
		$materials = {"": fallback};
	};
	$__string = @() "InstanceController(" + $url + ") " + $skeletons + " " + $materials;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$skeletons.(symbol)();
		$materials.(symbol)();
		$materials.each(@(key, value) {
			if (key != "") value.(symbol)();
		});
	};
	$build = @(resolve, shaders) {
		$_controller = resolve($url);
		$_controller.build(resolve);
		joints = resolve($_controller.joints["JOINT"]);
		joints.build(resolve);
		matrices = resolve($_controller.joints["INV_BIND_MATRIX"]);
		matrices.build(resolve);
		$_ibms = [];
		$_joints = [];
		$_joint2matrix = {};
		$_ibms.push($_controller.bind_shape_matrix);
		$_joints.push($_joints0 = Object());
		$_joints0.vertex = Matrix();
		for (i = 0; i < joints.count; i = i + 1) {
			$_ibms.push(matrices.matrix(i) * $_controller.bind_shape_matrix);
			$_joints.push($_joint2matrix[joints[i][0]] = Object());
		}
		$_vertices = Bytes((joints.count + 1) * 16 * gl.Float32Array.BYTES_PER_ELEMENT);
		$skeletons.each(@(x) {
			skeleton = resolve(x);
			skeleton.build(resolve, shaders);
			skeleton._controllers.push($);
		}[$]);
		$_shaders = {};
		$materials.each(@(key, value) {
			value.build(resolve);
			shader = Object();
			shader.model = value.model();
			shader.shader = shader.model.skin_shader(shaders, joints.count + 1, $_controller._weights_per_vertex);
			shader.bind_vertex_inputs = value.bind_vertex_inputs;
			$_shaders[key] = shader;
		}[$]);
		$_primitives = [];
		$_controller._primitives.each(@(x) {
			shader = $_shaders[x.material];
			others = {};
			x._others.each(@(x) {
				key = x[0];
				if (shader.bind_vertex_inputs.has(key)) key = shader.bind_vertex_inputs[key];
				others[key] = x[1];
			});
			uniforms = glshaders.Uniforms();
			uniforms.vertices = $_vertices;
			$_primitives.push(@(projection) {
				uniforms.projection = projection;
				shader.model.setup(uniforms, $_attributes, others);
				shader.shader(uniforms, $_attributes, $mode, 0, $count * $unit);
			}[x]);
		}[$]);
	};
	$render = @(projection) {
		n = $_ibms.size();
		stride = 16 * gl.Float32Array.BYTES_PER_ELEMENT;
		for (i = 0; i < n; i = i + 1) ($_joints[i].vertex * $_ibms[i]).bytes.copy(0, stride, $_vertices, i * stride);
		primitives = $_primitives;
		n = primitives.size();
		for (i = 0; i < n; i = i + 1) primitives[i](projection);
	};
};

Sampler = Class() :: WithTree :: @{
	Iterator = Class() :: @{
		$__initialize = @(sampler) {
			$input = sampler._input;
			$output = sampler._output;
			$i = 0;
			$channels = [];
		};
		$duration = @{
			n = $input.count;
			return n > 0 ? $input[n - 1][0] : 0.0;
		};
		$set = @(value) $channels.each(@(x) x(value));
		$forward = @(t) {
			n = $input.count;
			if (n <= 0) return $set(0.0);
			i = $i;
			while (i < n && $input[i][0] <= t) i = i + 1;
			if (i <= 0) return $set($output[0][0]);
			$i = i - 1;
			y0 = $output[$i][0];
			if (i >= n) return $set(y0);
			x0 = $input[$i][0];
			x1 = $input[i][0];
			y1 = $output[i][0];
			x1x0 = x1 - x0;
			$set(y0 * ((x1 - t) / x1x0) + y1 * ((t - x0) / x1x0));
		};
		$rewind = @(t) {
			$i = 0;
			$forward(t);
		};
	};

	$__initialize = @{
		$inputs = {};
		$built = false;
	};
	$__string = @() "Sampler(" + $id + ") " + $inputs;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$inputs.(symbol)();
	};
	$build = @(resolve) {
		if ($built) return;
		$_input = resolve($inputs["INPUT"]);
		$_input.build(resolve);
		$_output = resolve($inputs["OUTPUT"]);
		$_output.build(resolve);
		$_interpolation = resolve($inputs["INTERPOLATION"]);
		$_interpolation.build(resolve);
		$built = true;
	};
	$iterator = @() Iterator($);
};

Channel = Class() :: @{
	$__initialize = @() $built = false;
	$__string = @() "Channel(" + $source + ") {target: " + $target + "}";
	$build = @(resolve) {
		if ($built) return;
		$_source = resolve($source);
		$_source.build(resolve);
		path = [];
		parse_texts($target, @(x) x == 0x2f, path.push);
		$_node = resolve("#" + path[0]);
		i = find_index(path[1], 0, @(x) x == 0x28);
		sid = path[1].substring(0, i);
		if ($_node.sids.has(sid)) {
			target = $_node.sids[sid];
			if (i < path[1].size()) {
				i = i + 1;
				j = find_index(path[1], i, @(x) x == 0x29);
				column = Integer(path[1].substring(i, j - i));
				i = find_index(path[1], j + 1, @(x) x == 0x28) + 1;
				j = find_index(path[1], i, @(x) x == 0x29);
				row = Integer(path[1].substring(i, j - i));
				ij = row * 4 + column;
				$__call = ij == 15 ? @(value) {} : @(value) target.v[ij] = value;
			} else {
				$__call = @(value) value.bytes.copy(0, 16 * gl.Float32Array.BYTES_PER_ELEMENT, target.bytes, 0);
			}
		}
		$built = true;
	};
	$__call = @(value) {};
};

Animation = Class() :: WithTree :: @{
	$__initialize = @{
		$animations = [];
		$sources = [];
		$samplers = [];
		$channels = [];
	};
	$__string = @() "Animation(" + $id + ") " + $animations + " " + $samplers + " " + $channels;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$animations.(symbol)();
		$animations.each(@(x) x.(symbol)());
		$sources.(symbol)();
		$sources.each(@(x) x.(symbol)());
		$samplers.(symbol)();
		$samplers.each(@(x) x.(symbol)());
		$channels.(symbol)();
		$channels.each(@(x) x.(symbol)());
	};
	$iterators = @(resolve, iterators, use) {
		$animations.each(@(x) x.iterators(resolve, iterators, use));
		$channels.each(@(x) {
			x.build(resolve);
			if (!use(x)) return;
			iterator = iterators.has(x._source) ? iterators[x._source] : (iterators[x._source] = x._source.iterator());
			iterator.channels.push(x);
		});
	};
};

VisualScene = Class() :: WithTree :: @{
	$__initialize = @{
		$nodes = [];
		$built = false;
	};
	$__string = @() "VisualScene(" + $id + ") " + $nodes;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$nodes.(symbol)();
		$nodes.each(@(x) x.(symbol)());
	};
	$build = @(resolve, shaders) {
		if ($built) return;
		$nodes.each(@(x) x.build(resolve, shaders));
		ancestors = {};
		controllers = $_controllers = [];
		$nodes.each(@(x) x.postbuild(ancestors, controllers));
		$built = true;
	};
	$render = @(projection, viewing) {
t0 = time.now();
		joints = [];
		nodes = $nodes;
		n = nodes.size();
		for (i = 0; i < n; i = i + 1) nodes[i].render(projection, viewing, joints);
if (print_time) print("\tnodes: " + (time.now() - t0));
t0 = time.now();
		controllers = $_controllers;
		n = controllers.size();
		for (i = 0; i < n; i = i + 1) controllers[i].render(projection);
if (print_time) print("\tcontrollers: " + (time.now() - t0));
	};
};

InstanceVisualScene = Class() :: @{
	$__string = @() "InstanceVisualScene(" + $url + ")";
	$build = @(resolve, shaders) {
		$_scene = resolve($url);
		$_scene.build(resolve, shaders);
	};
	$render = @(projection, viewing) $_scene.render(projection, viewing);
};

Scene = Class() :: WithTree :: @{
	$__string = @() "Scene " + $instance_visual_scene;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$instance_visual_scene.(symbol)();
	};
	$build = @(resolve, shaders) $instance_visual_scene.build(resolve, shaders);
	$render = @(projection, viewing) $instance_visual_scene.render(projection, viewing);
};

Document = Class() :: WithTree :: @{
	$__initialize = @{
		$library_animations = {};
		$library_controllers = {};
		$library_effects = {};
		$library_geometries = {};
		$library_images = {};
		$library_materials = {};
		$library_nodes = {};
		$library_visual_scenes = {};
	};
	$__string = @() "Document " + $scene;
	$tree = @(symbol) {
		:$^.(symbol)[$]();
		$instance_material_fallback.(symbol)();
		$ids.(symbol)();
		$asset.(symbol)();
		$asset.unit.(symbol)();
		$library_animations.(symbol)();
		$library_animations.each(@(key, value) value.(symbol)());
		$library_controllers.(symbol)();
		$library_controllers.each(@(key, value) value.(symbol)());
		$library_effects.(symbol)();
		$library_effects.each(@(key, value) value.(symbol)());
		$library_geometries.(symbol)();
		$library_geometries.each(@(key, value) value.(symbol)());
		$library_images.(symbol)();
		$library_images.each(@(key, value) value.(symbol)());
		$library_materials.(symbol)();
		$library_materials.each(@(key, value) value.(symbol)());
		$library_nodes.(symbol)();
		$library_nodes.each(@(key, value) value.(symbol)());
		$library_visual_scenes.(symbol)();
		$library_visual_scenes.each(@(key, value) value.(symbol)());
		$scene.(symbol)();
	};
	$destroy = @{
		$library_controllers.each(@(key, value) value.destroy());
		$library_effects.each(@(key, value) value.destroy());
		$library_geometries.each(@(key, value) value.destroy());
	};
	$resolve = @(id) $ids[id.substring(1)];
	$build = @(shaders) $scene.build($resolve, shaders);
	$render = @(projection, viewing) $scene.render(projection, viewing);
	$iterators = @(use = @(x) true) {
		iterators = {};
		$library_animations.each((@(key, value) value.iterators($resolve, iterators, use))[$]);
		iterators;
	};
};

$load = @(source) {
	reader = Reader(source);
	ids = {};
	read_asset_x_extra = @(callback) {
		reader.read_next();
		if (reader.is_start_element("asset")) reader.read_element_text();
		callback();
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
	};
	read_asset_0x_extra = @(name, callback) {
		if (reader.is_empty_element()) return reader.read_next();
		read_asset_x_extra(@{
			while (reader.is_start_element(name)) callback();
		});
	};
	read_asset_1x_extra = @(name, callback) read_asset_x_extra(@{
		reader.check_start_element(name);
		while (true) {
			callback();
			if (!reader.is_start_element(name)) break;
		}
	});
	read_optional_integer = @(name, default) {
		x = reader.get_attribute(name);
		x == "" ? default : Integer(x);
	};
	read_matrix = @{
		xs = parse_array(gl.Float32Array, Float, 16, reader.read_element_text());
		m = Matrix4();
		for (i = 0; i < 16; i = i + 1) m.v[i] = xs[i];
		m;
	};
	read_empty_elements = @(name, callback) {
		for (; reader.is_start_element(name); reader.read_element_text()) callback();
	};
	read_array = @(type, parse) @(x) {
		count = Integer(reader.get_attribute("count"));
		id = reader.get_attribute("id");
		array = parse_array(type, parse, count, reader.read_element_text());
		array.id = id;
		if (id != "") ids[id] = array;
	};
	source_elements = {
		"Name_array": @(x) {
			count = Integer(reader.get_attribute("count"));
			id = reader.get_attribute("id");
			array = [];
			parse_texts(reader.read_element_text(), is_whitespace, array.push);
			if (array.size() != count) throw Throwable("wrong count");
			array.id = id;
			if (id != "") ids[id] = array;
		},
		"float_array": read_array(gl.Float32Array, Float),
		"int_array": read_array(gl.Int32Array, Integer),
		"technique_common": @(x) {
			reader.read_next();
			reader.check_start_element("accessor");
			x.count = Integer(reader.get_attribute("count"));
			x.offset = read_optional_integer("offset", 0);
			x.source = reader.get_attribute("source");
			x.stride = read_optional_integer("stride", 1);
			reader.read_next();
			params = [];
			read_empty_elements("param", @{
				param = Object();
				param.name = reader.get_attribute("name");
				param.type = reader.get_attribute("type");
				params.push(param);
			});
			if (params.size() > 1) {
				x.params = [];
				i = 0;
				params.each(@(param) {
					if (param.name != "") x.params.push(i);
					:i = i + 1;
				});
			} else {
				x.params = [0];
				if (params.size() == 1 && params[0].type == "float4x4") x.__get_at = x.get_matrix_at;
			}
			reader.end_element();
			reader.end_element();
		}
	};
	read_source = @{
		source = Source();
		source.id = reader.get_attribute("id");
		reader.parse_elements(source_elements, source);
		ids[source.id] = source;
		source;
	};
	asset_elements = {
		"unit": @(x) {
			x.unit.meter = Float(reader.get_attribute("meter"));
			x.unit.name = reader.get_attribute("name");
			reader.read_element_text();
		},
		"up_axis": @(x) x.up_axis = reader.read_element_text()
	};
	read_animation = null;
	animation_elements = {
		"animation": @(x) x.animations.push(read_animation()),
		"source": @(x) x.sources.push(read_source()),
		"sampler": @(x) {
			sampler = Sampler();
			sampler.id = reader.get_attribute("id");
			reader.read_next();
			read_empty_elements("input", @() sampler.inputs[reader.get_attribute("semantic")] = reader.get_attribute("source"));
			reader.end_element();
			x.samplers.push(sampler);
			if (sampler.id != "") ids[sampler.id] = sampler;
		},
		"channel": @(x) {
			channel = Channel();
			channel.source = reader.get_attribute("source");
			channel.target = reader.get_attribute("target");
			reader.read_element_text();
			x.channels.push(channel);
		}
	};
	read_animation = @{
		animation = Animation();
		animation.id = reader.get_attribute("id");
		reader.parse_elements(animation_elements, animation);
		if (animation.id != "") ids[animation.id] = animation;
		animation;
	};
	read_skin = @{
		skin = Skin();
		skin.source = reader.get_attribute("source");
		reader.read_next();
		if (reader.is_start_element("bind_shape_matrix")) skin.bind_shape_matrix = read_matrix();
		for (i = 0; i < 3; i = i + 1) {
			reader.check_start_element("source");
			skin.sources.push(read_source());
		}
		while (reader.is_start_element("source")) skin.sources.push(read_source());
		reader.check_start_element("joints");
		reader.read_next();
		read_empty_elements("input", @() skin.joints[reader.get_attribute("semantic")] = reader.get_attribute("source"));
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
		reader.check_start_element("vertex_weights");
		count = Integer(reader.get_attribute("count"));
		reader.read_next();
		read_empty_elements("input", @() skin.vertex_weights[reader.get_attribute("semantic")] = '(skin.vertex_weights.size(), reader.get_attribute("source")));
		reader.check_start_element("vcount");
		vcount = parse_array(gl.Int32Array, Integer, count, reader.read_element_text());
		n = 0;
		for (i = 0; i < count; i = i + 1) n = n + vcount[i];
		reader.check_start_element("v");
		skin.vertex_weights.count = count;
		skin.vertex_weights.vcount = vcount;
		skin.vertex_weights.v = parse_array(gl.Int32Array, Integer, n * skin.vertex_weights.size(), reader.read_element_text());
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
		skin;
	};
	profile_COMMON_newparam_surface_elements = {
		"init_from": @(x) {
			x.mip = reader.get_attribute("mip");
			x.slice = reader.get_attribute("slice");
			x.face = reader.get_attribute("face");
			x.from = reader.read_element_text();
		},
		"format": @(x) x.format = reader.read_element_text()
	};
	profile_COMMON_newparam_sampler2D_elements = {
		"source": @(x) x.source = reader.read_element_text(),
		"minfilter": @(x) x.minfilter = reader.read_element_text(),
		"magfilter": @(x) x.magfilter = reader.read_element_text()
	};
	profile_COMMON_newparam_elements = {
		"surface": @(x) {
			x.x = Surface();
			x.x.type = reader.get_attribute("type");
			reader.parse_elements(profile_COMMON_newparam_surface_elements, x.x);
		},
		"sampler2D": @(x) {
			x.x = Sampler2D();
			reader.parse_elements(profile_COMMON_newparam_sampler2D_elements, x.x);
		}
	};
	read_common_color_or_texture = @{
		reader.read_next();
		if (reader.is_start_element("color")) {
			xs = parse_array(gl.Float32Array, Float, 4, reader.read_element_text());
			value = CommonColor(xs[0], xs[1], xs[2], xs[3]);
		} else if (reader.is_start_element("param")) {
			value = CommonParam();
			value.ref = reader.get_attribute("ref");
			reader.read_element_text();
		} else if (reader.is_start_element("texture")) {
			value = Texture();
			value.texture = reader.get_attribute("texture");
			value.texcoord = reader.get_attribute("texcoord");
			reader.read_element_text();
		}
		reader.end_element();
		value;
	};
	read_float_or_param = @{
		reader.read_next();
		if (reader.is_start_element("float")) {
			value = CommonFloat(Float(reader.read_element_text()));
		} else if (reader.is_start_element("param")) {
			value = CommonParam();
			value.ref = reader.get_attribute("ref");
			reader.read_element_text();
		}
		reader.end_element();
		value;
	};
	shader_elements = {
		"emission": @(x) x.emission = read_common_color_or_texture(),
		"ambient": @(x) x.ambient = read_common_color_or_texture(),
		"diffuse": @(x) x.diffuse = read_common_color_or_texture(),
		"specular": @(x) x.specular = read_common_color_or_texture(),
		"shininess": @(x) x.shininess = read_float_or_param(),
		"reflective": @(x) x.reflective = read_common_color_or_texture(),
		"reflectivity": @(x) x.reflectivity = read_float_or_param(),
		"transparent": @(x) x.transparent = read_common_color_or_texture(),
		"transparency": @(x) x.transparency = read_float_or_param(),
		"index_of_refraction": @(x) x.index_of_refraction = read_float_or_param()
	};
	profile_COMMON_technique_elements = {
		"blinn": @(x) {
			x.model = Blinn();
			reader.parse_elements(shader_elements, x.model);
		},
		"constant": @(x) {
			x.model = Constant();
			reader.parse_elements(shader_elements, x.model);
		},
		"lambert": @(x) {
			x.model = Lambert();
			reader.parse_elements(shader_elements, x.model);
		},
		"phong": @(x) {
			x.model = Phong();
			reader.parse_elements(shader_elements, x.model);
		}
	};
	effect_elements = {
		"profile_COMMON": @(x) {
			profile = ProfileCOMMON();
			profile.id = reader.get_attribute("id");
			read_asset_x_extra(@{
				while (reader.is_start_element("image")) reader.read_element_text();
				while (reader.is_start_element("newparam")) {
					newparam = Object();
					sid = reader.get_attribute("sid");
					reader.parse_elements(profile_COMMON_newparam_elements, newparam);
					newparam.x.sid = sid;
					profile.sids[sid] = newparam.x;
					profile.newparams.push(newparam.x);
				}
				reader.check_start_element("technique");
				technique = TechniqueFX();
				technique.id = reader.get_attribute("id");
				technique.sid = reader.get_attribute("sid");
				reader.parse_elements(profile_COMMON_technique_elements, technique);
				profile.sids[technique.sid] = technique;
				profile.technique = technique;
				if (technique.id != "") ids[technique.id] = technique;
			});
			x.profiles.push(profile);
			if (profile.id != "") ids[profile.id] = profile;
		}
	};
	read_primitive_common = @(type, callback) {
		primitive = type();
		primitive.count = Integer(reader.get_attribute("count"));
		primitive.material = reader.get_attribute("material");
		reader.read_next();
		stride = 0;
		read_empty_elements("input", @{
			input = Input();
			input.offset = Integer(reader.get_attribute("offset"));
			if (input.offset + 1 > stride) :stride = input.offset + 1;
			input.semantic = reader.get_attribute("semantic");
			input.source = reader.get_attribute("source");
			set = reader.get_attribute("set");
			if (set == "") {
				primitive.inputs[input.semantic] = input;
			} else {
				input.set = Integer(set);
				primitive.inputs['(input.semantic, input.set)] = input;
			}
		});
		primitive.stride = stride;
		callback(primitive);
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
		primitive;
	};
	read_primitive = @(type) read_primitive_common(type, @(primitive) {
		if (reader.is_start_element("p")) primitive.indices = parse_array(gl.Int32Array, Integer, primitive.count * primitive.stride * 3, reader.read_element_text());
	});
	read_polylist = @() read_primitive_common(Triangles, @(primitive) {
		count = primitive.count;
		stride = primitive.stride;
		reader.check_start_element("vcount");
		vcount = parse_array(gl.Int32Array, Integer, count, reader.read_element_text());
		n = m = 0;
		for (i = 0; i < count; i = i + 1) {
			n = n + vcount[i];
			m = m + (vcount[i] - 2) * 3;
		}
		reader.check_start_element("p");
		indices = parse_array(gl.Int32Array, Integer, n * stride, reader.read_element_text());
		array = gl.Int32Array(Bytes(m * stride * gl.Int32Array.BYTES_PER_ELEMENT));
		n = m = triangles = 0;
		for (i = 0; i < count; i = i + 1) {
			c = vcount[i] - 2;
			for (j = 0; j < c; j = j + 1) {
				for (k = 0; k < stride; k = k + 1) {
					array[m + j * 3 * stride + k] = indices[n + k];
					array[m + (j * 3 + 1) * stride + k] = indices[n + (j + 1) * stride + k];
					array[m + (j * 3 + 2) * stride + k] = indices[n + (j + 2) * stride + k];
				}
			}
			n = n + vcount[i] * stride;
			m = m + c * 3 * stride;
			triangles = triangles + c;
		}
		primitive.count = triangles;
		primitive.indices = array;
	});
	mesh_elements = {
		"source": @(x) x.sources.push(read_source()),
		"vertices": @(x) {
			x.vertices = Object();
			x.vertices.id = reader.get_attribute("id");
			reader.read_next();
			x.vertices.inputs = {};
			read_empty_elements("input", @() x.vertices.inputs[reader.get_attribute("semantic")] = reader.get_attribute("source"));
			while (reader.is_start_element("extra")) reader.read_element_text();
			reader.end_element();
			ids[x.vertices.id] = x.vertices;
		},
		"lines": @(x) x.primitives.push(read_primitive(Lines)),
		"triangles": @(x) x.primitives.push(read_primitive(Triangles)),
		"polylist": @(x) x.primitives.push(read_polylist())
	};
	geometry_elements = {
		"mesh": @(x) {
			x.x = Mesh();
			reader.parse_elements(mesh_elements, x.x);
		}
	};
	read_instance_material = @(x) {
		material = InstanceMaterial();
		material.target = reader.get_attribute("target");
		material.symbol = reader.get_attribute("symbol");
		b = reader.is_empty_element();
		reader.read_next();
		if (!b) {
			while (reader.is_start_element("bind")) reader.read_element_text();
			while (reader.is_start_element("bind_vertex_input")) {
				semantic = reader.get_attribute("semantic");
				input_semantic = reader.get_attribute("input_semantic");
				input_set = Integer(reader.get_attribute("input_set"));
				reader.read_element_text();
				material.bind_vertex_inputs[input_semantic] = semantic;
				material.bind_vertex_inputs['(input_semantic, input_set)] = semantic;
			}
			while (reader.is_start_element("extra")) reader.read_element_text();
			reader.end_element();
		}
		x[material.symbol] = material;
	};
	read_bind_material = @(x) {
		reader.read_next();
		while (reader.is_start_element("param")) reader.read_element_text();
		reader.check_start_element("technique_common");
		reader.read_next();
		reader.check_start_element("instance_material");
		read_instance_material(x.materials);
		while (reader.is_start_element("instance_material")) read_instance_material(x.materials);
		reader.end_element();
		while (reader.is_start_element("technique")) reader.read_element_text();
		while (reader.is_start_element("extra")) reader.read_element_text();
		reader.end_element();
	};
	instance_material_fallback = InstanceMaterialFallback();
	read_node = null;
	node_elements = {
		"matrix": @(x) {
			sid = reader.get_attribute("sid");
			transform = Matrix(read_matrix());
			transform.sid = sid;
			if (sid != "") x.sids[sid] = transform;
			x.transforms.push(transform);
		},
		"rotate": @(x) {
			sid = reader.get_attribute("sid");
			xs = parse_array(gl.Float32Array, Float, 4, reader.read_element_text());
			transform = Rotate(xs[0], xs[1], xs[2], xs[3] * math.PI / 180.0);
			transform.sid = sid;
			if (sid != "") x.sids[sid] = transform;
			x.transforms.push(transform);
		},
		"scale": @(x) {
			sid = reader.get_attribute("sid");
			xs = parse_array(gl.Float32Array, Float, 3, reader.read_element_text());
			transform = Scale(xs[0], xs[1], xs[2]);
			transform.sid = sid;
			if (sid != "") x.sids[sid] = transform;
			x.transforms.push(transform);
		},
		"translate": @(x) {
			sid = reader.get_attribute("sid");
			xs = parse_array(gl.Float32Array, Float, 3, reader.read_element_text());
			transform = Translate(xs[0], xs[1], xs[2]);
			transform.sid = sid;
			if (sid != "") x.sids[sid] = transform;
			x.transforms.push(transform);
		},
		"instance_controller": @(x) {
			controller = InstanceController(instance_material_fallback);
			controller.url = reader.get_attribute("url");
			if (reader.is_empty_element()) {
				reader.read_next();
			} else {
				reader.read_next();
				while (reader.is_start_element("skeleton")) controller.skeletons.push(reader.read_element_text());
				if (reader.is_start_element("bind_material")) read_bind_material(controller);
				while (reader.is_start_element("extra")) reader.read_element_text();
				reader.end_element();
			}
			x.controllers.push(controller);
		},
		"instance_geometry": @(x) {
			geometry = InstanceGeometry(instance_material_fallback);
			geometry.url = reader.get_attribute("url");
			if (reader.is_empty_element()) {
				reader.read_next();
			} else {
				reader.read_next();
				if (reader.is_start_element("bind_material")) read_bind_material(geometry);
				while (reader.is_start_element("extra")) reader.read_element_text();
				reader.end_element();
			}
			x.geometries.push(geometry);
		},
		"instance_node": @(x) {
			x.instance_nodes.push(reader.get_attribute("url"));
			reader.read_element_text();
		},
		"node": @(x) x.nodes.push(read_node())
	};
	read_node = @{
		node = Node();
		node.id = reader.get_attribute("id");
		node.sid = reader.get_attribute("sid");
		node.joint = reader.get_attribute("type") == "JOINT";
		node.layer = reader.get_attribute("layer");
		reader.parse_elements(node_elements, node);
		if (node.id != "") ids[node.id] = node;
		node;
	};
	root_elements = {
		"asset": @(x) {
			x.asset = Object();
			x.asset.unit = Object();
			x.asset.unit.meter = 1.0;
			x.asset.unit.name = "meter";
			reader.parse_elements(asset_elements, x.asset);
		},
		"library_animations": @(x) {
			read_asset_1x_extra("animation", @{
				animation = read_animation();
				if (animation.id != "") x.library_animations[animation.id] = animation;
			});
		},
		"library_controllers": @(x) {
			read_asset_1x_extra("controller", @{
				id = reader.get_attribute("id");
				controller = null;
				read_asset_x_extra(@{
					if (reader.is_start_element("skin"))
						:controller = read_skin();
					else if (reader.is_start_element("morph"))
						reader.read_element_text();
				});
				controller.id = id;
				ids[id] = controller;
				x.library_controllers[id] = controller;
			});
		},
		"library_effects": @(x) {
			read_asset_1x_extra("effect", @{
				effect = Effect();
				effect.id = reader.get_attribute("id");
				reader.parse_elements(effect_elements, effect);
				ids[effect.id] = effect;
				x.library_effects[effect.id] = effect;
			});
		},
		"library_geometries": @(x) {
			read_asset_1x_extra("geometry", @{
				geometry = Object();
				id = reader.get_attribute("id");
				reader.parse_elements(geometry_elements, geometry);
				geometry.x.id = id;
				if (id == "") return;
				ids[id] = geometry.x;
				x.library_geometries[id] = geometry.x;
			});
		},
		"library_images": @(x) {
			#read_asset_1x_extra("image", @{
			read_asset_0x_extra("image", @{
				image = Image();
				image.id = reader.get_attribute("id");
				read_asset_x_extra(@{
					if (reader.is_start_element("data")) {
						image.type = "data";
						image.value = reader.read_element_text();
					} else if (reader.is_start_element("init_from")) {
						image.type = "path";
						path = io.Path(source) / ".." / reader.read_element_text();
						image.value = path.__string();
					}
				});
				if (image.id == "") return;
				ids[image.id] = image;
				x.library_images[image.id] = image;
			});
		},
		"library_materials": @(x) {
			read_asset_1x_extra("material", @{
				material = Material();
				material.id = reader.get_attribute("id");
				read_asset_x_extra(@{
					reader.check_start_element("instance_effect");
					material.instance_effect = reader.get_attribute("url");
					reader.read_element_text();
				});
				if (material.id == "") return;
				ids[material.id] = material;
				x.library_materials[material.id] = material;
			});
		},
		"library_nodes": @(x) {
			read_asset_1x_extra("node", @{
				node = read_node();
				if (node.id != "") x.library_nodes[node.id] = node;
			});
		},
		"library_visual_scenes": @(x) {
			read_asset_1x_extra("visual_scene", @{
				scene = VisualScene();
				scene.id = reader.get_attribute("id");
				read_asset_x_extra(@{
					reader.check_start_element("node");
					while (true) {
						scene.nodes.push(read_node());
						if (!reader.is_start_element("node")) break;
					}
					while (reader.is_start_element("evaluate_scene")) reader.read_element_text();
				});
				if (scene.id == "") return;
				ids[scene.id] = scene;
				x.library_visual_scenes[scene.id] = scene;
			});
		},
		"scene": @(x) {
			x.scene = Scene();
			reader.read_next();
			while (reader.is_start_element("instance_physics_scene")) reader.read_element_text();
			if (reader.is_start_element("instance_visual_scene")) {
				x.scene.instance_visual_scene = InstanceVisualScene();
				x.scene.instance_visual_scene.url = reader.get_attribute("url");
				reader.read_element_text();
			}
			while (reader.is_start_element("extra")) reader.read_element_text();
			reader.end_element();
		}
	};
	try {
		reader.read_next();
		reader.move_to_tag();
		reader.check_start_element("COLLADA");
		x = Document();
		x.instance_material_fallback = instance_material_fallback;
		x.ids = ids;
		x.version = reader.get_attribute("version");
		reader.parse_elements(root_elements, x);
		x;
	} finally {
		reader.free();
	}
};
