math = Module("math");
gl = Module("gl");

Float32Array = gl.Float32Array;
SingularMatrixException = Class(Throwable);

Matrix = @(N) Class() :: @{
	$N = N;
	NN = N * N;
	BPE = Float32Array.BYTES_PER_ELEMENT;
	NBPE = N * BPE;
	NNBPE = NN * BPE;
	I = Bytes(NNBPE);
	@{
		v = Float32Array(I);
		for (i = 0; i < N; i = i + 1) {
			r = i * N;
			for (j = 0; j < i; j = j + 1) v[r + j] = 0.0;
			v[r + i] = 1.0;
			for (j = i + 1; j < N; j = j + 1) v[r + j] = 0.0;
		}
	}();
	Icopy = I.copy;
	LU = Class() :: @{
		Row = Class() :: @{
			$__initialize = @(bytes, i) {
				$i = i;
				$w = 0.0;
				m = Float32Array(bytes, i * NBPE, N);
				for (i = 0; i < N; i = i + 1) m[i] = 0.0;
				$m = m;
			};
			$initialize = @(ds, i) {
				i = i * N;
				m = $m;
				m[0] = ds[i];
				d = math.fabs(m[0]);
				for (j = 1; j < N; j = j + 1) {
					m[j] = ds[i + j];
					d0 = math.fabs(m[j]);
					if (d0 > d) d = d0;
				}
				if (d == 0.0) return false;
				$w = 1.0 / d;
				true;
			};
			$pivot = @(i) $w * math.fabs($m[i]);
			$erase = @(d, i, ds) {
				m = $m;
				m[i] = m[i] / d;
				for (j = i + 1; j < N; j = j + 1) m[j] = m[j] - m[i] * ds[j];
			};
		};

		$__initialize = @{
			bytes = Bytes(NNBPE);
			rows = [];
			for (i = 0; i < N; i = i + 1) rows.push(Row(bytes, i));
			$rows = rows;
		};
		$initialize = @(x) {
			for (i = 0; i < N; i = i + 1) if (!$rows[i].initialize(x, i)) return false;
			true;
		};
		$pivot = @(i) {
			rows = $rows;
			d = rows[i].pivot(i);
			p = i;
			for (j = i + 1; j < N; j = j + 1) {
				d0 = rows[j].pivot(i);
				if (d0 <= d) continue;
				d = d0;
				p = j;
			}
			p;
		};
		$erase = @(i, d) {
			rows = $rows;
			ds = rows[i].m;
			for (j = i + 1; j < N; j = j + 1) rows[j].erase(d, i, ds);
		};
		$decompose = @(x) {
			if (!$initialize(x)) return false;
			for (i = 0; i < N - 1; i = i + 1) {
				p = $pivot(i);
				if (p != i) {
					t = $rows[i];
					$rows[i] = $rows[p];
					$rows[p] = t;
				}
				d = $rows[i].m[i];
				if (d == 0.0) return false;
				$erase(i, d);
			}
			$rows[N - 1].m[N - 1] != 0.0;
		};
		$backsubstitute = @(x) {
			for (j = 0; j < N; j = j + 1) {
				for (i = 0; i < N; i = i + 1) {
					ds = $rows[i].m;
					d = $rows[i].i == j ? 1.0 : 0.0;
					for (k = 0; k < i; k = k + 1) d = d - ds[k] * x[k * N + j];
					x[i * N + j] = d;
				}
				while (true) {
					i = i - 1;
					ds = $rows[i].m;
					d = x[i * N + j];
					for (k = i + 1; k < N; k = k + 1) d = d - ds[k] * x[k * N + j];
					x[i * N + j] = d / ds[i];
					if (i <= 0) break;
				}
			}
		};
		$determinant = @(x) {
			if (!$initialize(x)) return 0.0;
			determinant = 1.0;
			for (i = 0; i < N - 1; i = i + 1) {
				p = $pivot(i);
				if (p != i) {
					t = $rows[i];
					$rows[i] = $rows[p];
					$rows[p] = t;
					determinant = -determinant;
				}
				d = $rows[i].m[i];
				if (d == 0.0) return 0.0;
				determinant = determinant * d;
				$erase(i, d);
			}
			determinant * $rows[N - 1].m[N - 1];
		};
	};

	$__initialize = @(value = 1.0) {
		$bytes = Bytes(NNBPE);
		v = Float32Array($bytes);
		if (value.: === Float) {
			Icopy(0, NNBPE, $bytes, 0);
			for (i = 0; i < N; i = i + 1) v[i * N + i] = value;
		} else if (N == value.N) {
			value.bytes.copy(0, NNBPE, $bytes, 0);
		} else if (N < value.N) {
			nBPE = value.N * BPE;
			bytes = value.bytes;
			for (i = 0; i < N; i = i + 1) bytes.copy(i * nBPE, NBPE, $bytes, i * NBPE);
		} else {
			n = value.N;
			nBPE = n * BPE;
			bytes = value.bytes;
			for (i = 0; i < n; i = i + 1) {
				rBPE = i * NBPE;
				bytes.copy(i * nBPE, nBPE, $bytes, rBPE);
				j = rBPE + nBPE;
				Icopy(j, NBPE - nBPE, $bytes, j);
			}
			i = n * NBPE;
			Icopy(i, NNBPE - i, $bytes, i);
		}
		$v = v;
	};
	$__string = @{
		v = $v;
		s = "";
		for (i = 0; i < N; i = i + 1) {
			r = i * N;
			t = "[";
			for (j = 0; j < N; j = j + 1) t = t + " " + v[r + j];
			s = s + t + "]\n";
		}
		s;
	};
	$__get_at = @(i) $v[i];
	$__equals = @(value) {
		v0 = $v;
		v1 = value.v;
		for (i = 0; i < NN; i = i + 1) if (v0[i] != v1[i]) return false;
		true;
	};
	$__not_equals = @(value) !$__equals(value);
	$negate = @{
		v = $v;
		for (i = 0; i < NN; i = i + 1) v[i] = -v[i];
		$;
	};
	$__minus = @() :$($).negate();
	$add = @(value) {
		v0 = $v;
		v1 = value.v;
		for (i = 0; i < NN; i = i + 1) v0[i] = v0[i] + v1[i];
		$;
	};
	$__add = @(value) :$($).add(value);
	$subtract = @(value) {
		v0 = $v;
		v1 = value.v;
		for (i = 0; i < NN; i = i + 1) v0[i] = v0[i] - v1[i];
		$;
	};
	$__subtract = @(value) :$($).subtract(value);
	multiply = @(v, v0, v1) {
		for (i = 0; i < N; i = i + 1) {
			r = i * N;
			for (j = 0; j < N; j = j + 1) {
				d = 0.0;
				for (k = 0; k < N; k = k + 1) d = d + v0[r + k] * v1[k * N + j];
				v[r + j] = d;
			}
		}
	};
	$multiply = @(value) {
		v = $v;
		if (value.: === Float)
			for (i = 0; i < NN; i = i + 1) v[i] = v[i] * value;
		else
			multiply(v, Tuple(*v), value.v);
		$;
	};
	$__multiply = @(value) {
		x = :$($);
		v = x.v;
		if (value.: === Float)
			for (i = 0; i < NN; i = i + 1) v[i] = v[i] * value;
		else
			multiply(v, $v, value.v);
		x;
	};
	$divide = @(value) $multiply(1.0 / value);
	$__divide = @(value) $__multiply(1.0 / value);
	$invert = @{
		lu = LU();
		if (!lu.decompose($v)) throw SingularMatrixException($__string());
		lu.backsubstitute($v);
		$;
	};
	$__complement = @() :$($).invert();
	$equals = @(value, epsilon) {
		v0 = $v;
		v1 = value.v;
		for (i = 0; i < NN; i = i + 1) if (math.fabs(v0[i] - v1[i]) > epsilon) return false;
		true;
	};
	$transpose = @{
		v = $v;
		n = N - 1;
		for (i = 0; i < n; i = i + 1) {
			r = i * N;
			for (j = i + 1; j < N; j = j + 1) {
				i0 = r + j;
				i1 = j * N + i;
				t = v[i0];
				v[i0] = v[i1];
				v[i1] = t;
			}
		}
		$;
	};
	$transposition = @() :$($).transpose();
	$determinant = @() LU().determinant($v);
};

Matrix3 = Matrix(3);
Matrix4 = Matrix(4);

Vector3 = Class() :: @{
	$__initialize = @(x, y, z) {
		$x = x;
		$y = y;
		$z = z;
	};
	$__string = @() "[" + $x + " " + $y + " " + $z + "]";
	$add = @(value) {
		$x = $x + value.x;
		$y = $y + value.y;
		$z = $z + value.z;
	};
	$subtract = @(value) {
		$x = $x - value.x;
		$y = $y - value.y;
		$z = $z - value.z;
	};
	$negate = @{
		$x = -$x;
		$y = -$y;
		$z = -$z;
	};
	$scale = @(value) {
		if (value.: === Float) {
			$x = $x * value;
			$y = $y * value;
			$z = $z * value;
		} else {
			$x = $x * value.x;
			$y = $y * value.y;
			$z = $z * value.z;
		}
	};
	$__equals = @(value) $x == value.x && $y == value.y && $z == value.z;
	$__not_equals = @(value) !$__equals(value);
	$equals = @(value, epsilon) {
		if (math.fabs($x - value.x) > epsilon) return false;
		if (math.fabs($y - value.y) > epsilon) return false;
		if (math.fabs($z - value.z) > epsilon) return false;
		true;
	};
	$absolute = @{
		$x = math.fabs($x);
		$y = math.fabs($y);
		$z = math.fabs($z);
	};
	$interpolate = @(value, t) {
		$x = (1.0 - t) * $x + t * value.x;
		$y = (1.0 - t) * $y + t * value.y;
		$z = (1.0 - t) * $z + t * value.z;
	};
	$__minus = @() Vector3(-$x, -$y, -$z);
	$__add = @(value) Vector3($x + value.x, $y + value.y, $z + value.z);
	$__subtract = @(value) Vector3($x - value.x, $y - value.y, $z - value.z);
	$__multiply = @(value) value.: === Float ? Vector3($x * value, $y * value, $z * value) : $x * value.x + $y * value.y + $z * value.z;
	$__divide = @(value) Vector3($x / value, $y / value, $z / value);
	$__xor = @(value) Vector3($y * value.z - $z * value.y, $z * value.x - $x * value.z, $x * value.y - $y * value.x);
	$length = @() math.sqrt($ * $);
	$normalize = @() $scale(1.0 / $length());
	$normalized = @() $ / $length();
	$angle = @(value) {
		d = $ * value / ($length() * value.length());
		math.acos(d < -1.0 ? -1.0 : d > 1.0 ? 1.0 : d);
	};
};

Vector4 = Class() :: @{
	$__initialize = @(x, y, z, w) {
		$x = x;
		$y = y;
		$z = z;
		$w = w;
	};
	$__string = @() "[" + $x + " " + $y + " " + $z + " " + $w + "]";
	$add = @(value) {
		$x = $x + value.x;
		$y = $y + value.y;
		$z = $z + value.z;
		$w = $w + value.w;
	};
	$subtract = @(value) {
		$x = $x - value.x;
		$y = $y - value.y;
		$z = $z - value.z;
		$w = $w - value.w;
	};
	$negate = @{
		$x = -$x;
		$y = -$y;
		$z = -$z;
		$w = -$w;
	};
	$scale = @(value) {
		$x = $x * value.x;
		$y = $y * value.y;
		$z = $z * value.z;
		$w = $w * value.w;
	};
	$__equals = @(value) $x == value.x && $y == value.y && $z == value.z && $w == value.w;
	$__not_equals = @(value) !$__equals(value);
	$equals = @(value, epsilon) {
		if (math.fabs($x - value.x) > epsilon) return false;
		if (math.fabs($y - value.y) > epsilon) return false;
		if (math.fabs($z - value.z) > epsilon) return false;
		if (math.fabs($w - value.w) > epsilon) return false;
		true;
	};
	$absolute = @{
		$x = math.fabs($x);
		$y = math.fabs($y);
		$z = math.fabs($z);
		$w = math.fabs($w);
	};
	$interpolate = @(value, t) {
		$x = (1.0 - t) * $x + t * value.x;
		$y = (1.0 - t) * $y + t * value.y;
		$z = (1.0 - t) * $z + t * value.z;
		$w = (1.0 - t) * $w + t * value.w;
	};
	$__minus = @() Vector4(-$x, -$y, -$z, -$w);
	$__add = @(value) Vector4($x + value.x, $y + value.y, $z + value.z, $w + value.w);
	$__subtract = @(value) Vector4($x - value.x, $y - value.y, $z - value.z, $w - value.w);
	$__multiply = @(value) value.: === Float ? Vector4($x * value, $y * value, $z * value, $w * value) : $x * value.x + $y * value.y + $z * value.z + $w * value.w;
	$__divide = @(value) Vector4($x / value, $y / value, $z / value, $w / value);
	$__xor = @(value) Vector3($y * value.z - $z * value.y, $z * value.x - $x * value.z, $x * value.y - $y * value.x);
	$length = @() math.sqrt($ * $);
	$normalize = @() $scale(1.0 / $length());
	$normalized = @() $ / $length();
	$angle = @(value) {
		d = $ * value / ($length() * value.length());
		math.acos(d < -1.0 ? -1.0 : d > 1.0 ? 1.0 : d);
	};
};

Matrix3__multiply = Matrix3.__multiply;
Matrix3.__multiply = @(value) {
	if (value.: === Vector3) {
		v = $v;
		Vector3(
			v[0] * value.x + v[1] * value.y + v[2] * value.z,
			v[3] * value.x + v[4] * value.y + v[5] * value.z,
			v[6] * value.x + v[7] * value.y + v[8] * value.z
		);
	} else {
		Matrix3__multiply[$](value);
	}
};

Matrix4__multiply = Matrix4.__multiply;
Matrix4.__multiply = @(value) {
	if (value.: === Vector3) {
		v = $v;
		Vector3(
			v[0] * value.x + v[1] * value.y + v[2] * value.z + v[3],
			v[4] * value.x + v[5] * value.y + v[6] * value.z + v[7],
			v[8] * value.x + v[9] * value.y + v[10] * value.z + v[11]
		);
	} else if (value.: === Vector4) {
		v = $v;
		Vector4(
			v[0] * value.x + v[1] * value.y + v[2] * value.z + v[3] * value.w,
			v[4] * value.x + v[5] * value.y + v[6] * value.z + v[7] * value.w,
			v[8] * value.x + v[9] * value.y + v[10] * value.z + v[11] * value.w,
			v[12] * value.x + v[13] * value.y + v[14] * value.z + v[15] * value.w
		);
	} else {
		Matrix4__multiply[$](value);
	}
};

Matrix4.translate = @(x, y, z) {
	v = $v;
	v[3] = v[0] * x + v[1] * y + v[2] * z + v[3];
	v[7] = v[4] * x + v[5] * y + v[6] * z + v[7];
	v[11] = v[8] * x + v[9] * y + v[10] * z + v[11];
	$;
};

Matrix4.scale = @(x, y, z) {
	v = $v;
	v[0] = v[0] * x;
	v[1] = v[1] * y;
	v[2] = v[2] * z;
	v[4] = v[4] * x;
	v[5] = v[5] * y;
	v[6] = v[6] * z;
	v[8] = v[8] * x;
	v[9] = v[9] * y;
	v[10] = v[10] * z;
	$;
};

Matrix4.rotate = @(axis, angle) {
	d = axis.length();
	if (d < 0.0000000001) return $;
	x = axis.x / d;
	y = axis.y / d;
	z = axis.z / d;
	s = math.sin(angle);
	c = math.cos(angle);
	d0 = 1.0 - c;
	zx = z * x;
	xy = x * y;
	yz = y * z;
	m = Matrix4();
	v = m.v;
	v[0] = d0 * x * x + c;
	v[1] = d0 * xy - s * z;
	v[2] = d0 * zx + s * y;
	v[4] = d0 * xy + s * z;
	v[5] = d0 * y * y + c;
	v[6] = d0 * yz - s * x;
	v[8] = d0 * zx - s * y;
	v[9] = d0 * yz + s * x;
	v[10] = d0 * z * z + c;
	$multiply(m);
};

Matrix4.frustum = @(left, right, bottom, top, near, far) {
	m = Matrix4(0.0);
	v = m.v;
	v[0] = 2.0 * near / (right - left);
	v[2] = (right + left) / (right - left);
	v[5] = 2.0 * near / (top - bottom);
	v[6] = (top + bottom) / (top - bottom);
	v[10] = -(far + near) / (far - near);
	v[11] = -2.0 * far * near / (far - near);
	v[14] = -1.0;
	$multiply(m);
};

Matrix4.orthographic = @(left, right, bottom, top, near, far) {
	m = Matrix4();
	v = m.v;
	v[0] = 2.0 / (right - left);
	v[3] = -(right + left) / (right - left);
	v[5] = 2.0 / (top - bottom);
	v[7] = -(top + bottom) / (top - bottom);
	v[10] = -2.0 / (far - near);
	v[11] = -(far + near) / (far - near);
	$multiply(m);
};

$Matrix3 = Matrix3;
$Matrix4 = Matrix4;
$Vector3 = Vector3;
$Vector4 = Vector4;

if ($ === Module("__main")) {
	system = Module("system");
	print = system.out.write_line;
	assert = @(x) if (!x) throw Throwable("Assertion failed.");;

	m = Matrix3();
	print(m);
	assert(m.equals(Matrix3(1.0), 0.0001));

	m = Matrix3(Matrix3(2.0));
	print(m);
	assert(m == Matrix3(2.0));

	m = Matrix3(Matrix4(2.0));
	print(m);
	assert(m == Matrix3(2.0));

	m = Matrix4(Matrix3(2.0));
	print(m);
	m1 = Matrix4(2.0);
	m1.v[15] = 1.0;
	assert(m.equals(m1, 0.0001));

	m = -Matrix3();
	print(m);
	assert(m.equals(Matrix3(-1.0), 0.0001));

	m = Matrix3() + Matrix3(2.0);
	print(m);
	assert(m.equals(Matrix3(3.0), 0.0001));

	m = Matrix3(3.0) - Matrix3();
	print(m);
	assert(m.equals(Matrix3(2.0), 0.0001));

	m = Matrix3() * 2.0;
	print(m);
	assert(m == Matrix3(2.0));

	m = Matrix3(2.0) * Matrix3(2.0);
	print(m);
	assert(m.equals(Matrix3(4.0), 0.0001));

	m = Matrix3(4.0) / 2.0;
	print(m);
	assert(m == Matrix3(2.0));

	m = Matrix3();
	m.v[0] = 0.0;
	m.v[1] = 1.0;
	m.v[3] = -1.0;
	m.v[4] = 0.0;
	m.invert();
	print(m);
	m1 = Matrix3();
	m1.v[0] = 0.0;
	m1.v[1] = -1.0;
	m1.v[3] = 1.0;
	m1.v[4] = 0.0;
	assert(m.equals(m1, 0.0001));

	m = Matrix3();
	m.v[2] = 2.0;
	m.transpose();
	print(m);
	m1 = Matrix3();
	m1.v[6] = 2.0;
	assert(m == m1);

	m = Matrix4().translate(1.0, 2.0, -3.0);
	print(m);
	m1 = Matrix4().translate(-1.0, -2.0, 3.0).invert();
	assert(m.equals(m1, 0.0001));
	v = m * Vector3(10.0, 20.0, 30.0);
	print(v);
	assert(v.equals(Vector3(11.0, 22.0, 27.0), 0.0001));

	m = Matrix4().scale(2.0, -4.0, 8.0);
	print(m);
	m1 = Matrix4().scale(0.5, -0.25, 0.125).invert();
	assert(m.equals(m1, 0.0001));
	v = m * Vector3(10.0, 20.0, 30.0);
	print(v);
	assert(v.equals(Vector3(20.0, -80.0, 240.0), 0.0001));

	m = Matrix4().rotate(Vector3(1.0, 2.0, -3.0), math.PI / 6.0);
	print(m);
	m1 = Matrix4().rotate(Vector3(1.0, 2.0, -3.0), -math.PI / 6.0).invert();
	assert(m.equals(m1, 0.0001));

	v = Matrix4().rotate(Vector3(0.0, 0.0, 1.0), math.PI / 6.0) * Vector3(10.0, 0.0, 30.0);
	print(v);
	assert(v.equals(Vector3(10.0 * math.cos(math.PI / 6.0), 10.0 * math.sin(math.PI / 6.0), 30.0), 0.0001));

	m = Matrix4().frustum(-2.0, 2.0, -1.0, 1.0, 1.0, 2.0);
	print(m);
	m1 = Matrix4(0.0);
	m1.v[0] = 0.5;
	m1.v[5] = 1.0;
	m1.v[10] = -3.0;
	m1.v[11] = -4.0;
	m1.v[14] = -1.0;
	assert(m.equals(m1, 0.0001));
	v = m * Vector4(10.0, 20.0, -30.0, 1.0);
	print(v);
	assert(v.equals(Vector4(5.0, 20.0, 86.0, 30.0), 0.0001));
}
