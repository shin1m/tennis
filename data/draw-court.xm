cairo = Module("cairo");

cairo.main(@{
	image = cairo.ImageSurface(cairo.Format.ARGB32, 1024, 2048);
	context = cairo.Context(image);
	context.set_source_rgb(0.125, 0.75, 0.375);
	context.rectangle(0, 0, 60 * 12, 120 * 12);
	context.fill();
	context.set_source_rgb(0.125, 0.25, 0.5);
	context.rectangle(12 * 12, 21 * 12, 36 * 12, 78 * 12);
	context.fill();
	context.set_source_rgb(1.0, 1.0, 1.0);
	context.rectangle(12 * 12, 21 * 12, 36 * 12, 2);
	context.rectangle(12 * 12, (21 + 78) * 12 - 2, 36 * 12, 2);
	context.rectangle((12 + 4) * 12 + 6, (21 + 18) * 12, 27 * 12, 2);
	context.rectangle((12 + 4) * 12 + 6, (21 + 60) * 12 - 2, 27 * 12, 2);
	context.rectangle(12 * 12, 21 * 12, 2, 78 * 12);
	context.rectangle((12 + 4) * 12 + 6, 21 * 12, 2, 78 * 12);
	context.rectangle((12 + 31) * 12 + 4, 21 * 12, 2, 78 * 12);
	context.rectangle((12 + 36) * 12 - 2, 21 * 12, 2, 78 * 12);
	context.rectangle((12 + 18) * 12 - 1, 21 * 12, 2, 6);
	context.rectangle((12 + 18) * 12 - 1, (21 + 18) * 12, 2, 42 * 12);
	context.rectangle((12 + 18) * 12 - 1, (21 + 78) * 12 - 6, 2, 6);
	context.fill();
	context.set_source_rgb(0.0, 0.0, 0.0);
	for (i = 0; i < 3 * 4 + 2; i = i + 1) context.rectangle(9 * 12, 125 * 12 + i * 3, 42 * 12, 1);
	for (i = 0; i < 42 * 4; i = i + 1) context.rectangle(9 * 12 + i * 3, 125 * 12, 1, 3 * 12 + 6);
	context.fill();
	context.set_source_rgb(1.0, 1.0, 1.0);
	context.rectangle(9 * 12, 125 * 12, 42 * 12, 2);
	context.rectangle((12 + 18) * 12 - 1, 125 * 12, 2, 3 * 12 + 6);
	context.rectangle(3 * 12, 121 * 12, 3 * 12, 7 * 12 + 6);
	context.fill();
	context.set_source_rgb(0.125, 0.25, 0.5);
	context.rectangle(0, 130 * 12, 60 * 12, 6 * 12);
	context.fill();
	context.set_source_rgb(0.125, 0.75, 0.375);
	context.rectangle(0, 140 * 12, 60 * 12, 12 * 12);
	context.fill();
	image.write_to_png("court.png");
});
