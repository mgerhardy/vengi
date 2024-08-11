/**
 * @file
 */

#include "PaintNetPalette.h"
#include "core/Color.h"
#include "engine-config.h"

namespace palette {

bool PaintNetPalette::load(const core::String &filename, io::SeekableReadStream &stream, palette::Palette &palette) {
	char line[2048];
	int n = 0;
	while (stream.readLine(sizeof(line), line)) {
		if (line[0] == ';') {
			continue;
		}
		core::RGBA argb = core::Color::fromHex(line);
		if (argb.r == 0) {
			continue;
		}
		palette.setColor(n++, core::RGBA(argb.g, argb.b, argb.a, argb.r));
	}

	return true;
}

// A palette usually consists of ninety six (96) colors. If there are less than this, the remaining color
// slots will be set to white (FFFFFFFF). If there are more, then the remaining colors will be ignored.
bool PaintNetPalette::save(const palette::Palette &palette, const core::String &filename,
						   io::SeekableWriteStream &stream) {
	stream.writeLine("; paint.net Palette File");
	stream.writeLine("; Generated by vengi " PROJECT_VERSION " github.com/vengi-voxel/vengi");
	stream.writeStringFormat(false, "; Palette Name: %s\n", palette.name().c_str());
	stream.writeStringFormat(false, "; Colors: %i\n", (int)palette.size());

	for (size_t i = 0; i < palette.size(); ++i) {
		const core::RGBA &rgba = palette.color(i);
		stream.writeStringFormat(false, "%02x%02x%02x%02x\n", rgba.a, rgba.r, rgba.g, rgba.b);
	}
	return true;
}

} // namespace palette
