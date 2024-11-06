/**
 * @file
 */

#include "FormatPrinter.h"
#include "core/Common.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "core/collection/DynamicArray.h"
#include "io/FormatDescription.h"
#include "palette/PaletteFormatDescription.h"
#include "voxelformat/VolumeFormat.h"
#include <SDL3/SDL_stdinc.h>

FormatPrinter::FormatPrinter(const io::FilesystemPtr &filesystem, const core::TimeProviderPtr &timeProvider)
	: Super(filesystem, timeProvider) {
	init(ORGANISATION, "formatprinter");
	_saveConfiguration = false;
}

app::AppState FormatPrinter::onConstruct() {
	registerArg("--palette").setDescription("Print the supported palettes");
	registerArg("--image").setDescription("Print the supported image");
	registerArg("--manpage").setDescription("Print the manpage entries for loading and saving");
	registerArg("--voxel").setDescription("Print the supported voxel formats");
	registerArg("--mimeinfo").setDescription("Generate the mimeinfo file for voxel formats");
	registerArg("--markdown").setDescription("Generate the markdown tables for voxel, image and palette formats");
	registerArg("--plist").setDescription("Generate the plist file for voxel formats");
	registerArg("--wix").setDescription("Generate the wix file for msi installers");
	registerArg("--magic").setDescription("Generate the magic file");
	return Super::onConstruct();
}

template<class T>
static void printJsonStringArray(const T &array) {
	int i = 0;
	for (const core::String &e : array) {
		if (i != 0) {
			Log::printf(",");
		}
		Log::printf("\"%s\"", e.c_str());
		++i;
	}
}

template<class T>
static void printJsonMagicArray(const T &array) {
	int i = 0;
	for (const core::String &e : array) {
		if (i != 0) {
			Log::printf(",");
		}
		Log::printf("{");
		if (SDL_isprint(e.first())) {
			Log::printf("\"type\": \"string\", \"value\": \"%s\"", e.c_str());
		} else {
			Log::printf("\"type\": \"bytes\", \"value\": \"");
			Log::printf("0x");
			for (size_t j = 0; j < e.size(); ++j) {
				Log::printf("%02X", (uint8_t)e[j]);
			}
			Log::printf("\"");
		}
		Log::printf("}");
		++i;
	}
}

static bool voxelSaveSupported(const io::FormatDescription &desc) {
	if ((desc.flags & FORMAT_FLAG_SAVE) == 0) {
		Log::debug("Format %s does not support saving", desc.name.c_str());
		return false;
	}
	for (const io::FormatDescription *d = voxelformat::voxelSave(); d->valid(); ++d) {
		int foundExtensionMatch = 0;
		for (const core::String &ext : d->exts) {
			if (!desc.matchesExtension(ext)) {
				continue;
			}
			if (desc.name == d->name) {
				Log::debug("Found save format %s", d->name.c_str());
				return true;
			}
			++foundExtensionMatch;
			Log::debug("Found save format by extension %s but it does not match by name %s vs %s", ext.c_str(),
					   desc.name.c_str(), d->name.c_str());
			break;
		}
		Log::debug("Found matches for %s with %i extensions", desc.name.c_str(), foundExtensionMatch);
		if (foundExtensionMatch == 1) {
			return true;
		}
	}
	return false;
}

app::AppState FormatPrinter::onRunning() {
	if (hasArg("--mimeinfo")) {
		// this is only for voxels
		printMimeInfo();
	} else if (hasArg("--markdown")) {
		printMarkdownTables();
	} else if (hasArg("--magic")) {
		printMagic();
	} else if (hasArg("--manpage")) {
		printManPageLoadSaveFormats();
	} else if (hasArg("--plist")) {
		printApplicationPlist();
	} else if (hasArg("--wix")) {
		printInstallerWix();
	} else {
		const bool palette = hasArg("--palette");
		const bool image = hasArg("--image");
		const bool voxel = hasArg("--voxel");
		if (!palette && !image && !voxel) {
			usage();
			return app::AppState::Cleanup;
		}
		printJson(palette, image, voxel);
	}
	return app::AppState::Cleanup;
}

core::String FormatPrinter::uniqueMimetype(const io::FormatDescription &desc) {
	// TODO: maybe add a mimetype to the format description
	core::String name = desc.name.toLower();
	core::string::replaceAllChars(name, ' ', '-');
	core::string::replaceAllChars(name, ':', '-');
	core::string::replaceAllChars(name, '.', '-');
	core::string::replaceAllChars(name, '/', '-');
	core::String mt = core::string::format("application/x-%s", name.c_str());
	if (_uniqueMimetypes.has(mt)) {
		mt += "-" + desc.mainExtension();
	}
	core::string::replaceAllChars(mt, '.', '-');
	_uniqueMimetypes.insert(mt);
	return mt;
}

void FormatPrinter::printManPageLoadSaveFormats() {
	core::DynamicArray<io::FormatDescription> formatDescriptions;
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		formatDescriptions.push_back(*desc);
	}
	formatDescriptions.sort(core::Greater<io::FormatDescription>());

	Log::printf(".SH LOAD\n");
	for (const io::FormatDescription &desc : formatDescriptions) {
		Log::printf(".TP\n");
		Log::printf("%s (", desc.name.c_str());
		int ext = 0;
		for (const core::String &e : desc.exts) {
			if (ext > 0) {
				Log::printf(", ");
			}
			Log::printf("*.%s", e.c_str());
			++ext;
		}
		Log::printf(")\n");
	}
	Log::printf("\n");

	Log::printf(".SH SAVE\n");
	for (const io::FormatDescription &desc : formatDescriptions) {
		if (!voxelSaveSupported(desc)) {
			continue;
		}
		Log::printf(".TP\n");
		Log::printf("%s (", desc.name.c_str());
		int ext = 0;
		for (const core::String &e : desc.exts) {
			if (ext > 0) {
				Log::printf(", ");
			}
			Log::printf("*.%s", e.c_str());
			++ext;
		}
		Log::printf(")\n");
	}
	Log::printf("\n");
}

void FormatPrinter::printMagic() {
	core::DynamicArray<io::FormatDescription> formatDescriptions;
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		formatDescriptions.push_back(*desc);
	}
	formatDescriptions.sort(core::Greater<io::FormatDescription>());

	for (const io::FormatDescription &desc : formatDescriptions) {
		if (desc.magics.empty()) {
			continue;
		}
		Log::printf("# %s\n", desc.name.c_str());
		const core::String &m = uniqueMimetype(desc);
		for (const core::String &magic : desc.magics) {
			if (SDL_isprint(magic.first())) {
				Log::printf("50 string 0 \"%s\"", magic.c_str());
			} else {
				Log::printf("50 byte 0 ");
				for (size_t i = 0; i < magic.size(); ++i) {
					Log::printf("%02X", (uint8_t)magic[i]);
				}
			}
			Log::printf(" %s\n", m.c_str());
		}
	}
}

void FormatPrinter::printMarkdownTables() {
	Log::printf("# Formats\n");
	Log::printf("\n");
	Log::printf("## Voxel formats\n");
	Log::printf("\n");
	Log::printf("> The [vengi](FormatSpec.md) format is the best supported format. Saving into any other format might lose several details "
		   "from your scene. This depends on the capabilities of the format and the completeness of the implementation "
		   "for supporting that particular format.\n");
	Log::printf("\n");
	Log::printf("| Name                       | Extension   | Loading | Saving | Thumbnails | Palette | Animations |\n");
	Log::printf("| :------------------------- | ----------- | ------- | ------ | ---------- | ------- | ---------- |\n");
	core::DynamicArray<io::FormatDescription> formatDescriptions;
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		formatDescriptions.push_back(*desc);
	}
	formatDescriptions.sort(core::Greater<io::FormatDescription>());
	for (const io::FormatDescription &desc : formatDescriptions) {
		if (voxelformat::isMeshFormat(desc)) {
			continue;
		}
		const bool screenshot = desc.flags & VOX_FORMAT_FLAG_SCREENSHOT_EMBEDDED;
		const bool palette = desc.flags & VOX_FORMAT_FLAG_PALETTE_EMBEDDED;
		const bool animation = desc.flags & VOX_FORMAT_FLAG_ANIMATION;
		const bool save = voxelSaveSupported(desc);
		Log::printf("| %-26s | %-11s | %-7s | %-6s | %-10s | %-7s | %-10s |\n", desc.name.c_str(),
			   desc.mainExtension().c_str(), "X", save ? "X" : " ", screenshot ? "X" : " ", palette ? "X" : " ",
			   animation ? "X" : " ");
	}
	Log::printf("\n");
	Log::printf("## Mesh formats\n");
	Log::printf("\n");
	Log::printf("| Name                       | Extension | Loading | Saving    | Animations |\n");
	Log::printf("| :------------------------- | --------- | ------- | --------- | ---------- |\n");
	for (const io::FormatDescription &desc : formatDescriptions) {
		if (!voxelformat::isMeshFormat(desc)) {
			continue;
		}
		core::String spec;
		const bool animation = desc.flags & VOX_FORMAT_FLAG_ANIMATION;
		const bool save = voxelSaveSupported(desc);
		Log::printf("| %-26s | %-9s | %-7s | %-9s | %-10s |\n", desc.name.c_str(), desc.mainExtension().c_str(), "X",
			   save ? "X" : " ", animation ? "X" : " ");
	}
	Log::printf("\nPoint cloud support for `ply` and `gtlf` is implemented, too.\n");
	Log::printf("\n");
	Log::printf("## Palettes\n");
	Log::printf("\n");
	Log::printf("| Name                            | Extension | Loading | Saving |\n");
	Log::printf("| :------------------------------ | --------- | ------- | ------ |\n");

	formatDescriptions.clear();
	for (const io::FormatDescription *desc = palette::palettes(); desc->valid(); ++desc) {
		formatDescriptions.push_back(*desc);
	}
	formatDescriptions.sort(core::Greater<io::FormatDescription>());
	for (const io::FormatDescription &desc : formatDescriptions) {
		const bool save = desc.flags & FORMAT_FLAG_SAVE;
		Log::printf("| %-31s | %-9s | X       | %c      |\n", desc.name.c_str(), desc.mainExtension().c_str(),
			   save ? 'X' : ' ');
	}

	Log::printf("\n> The `gpl` format also supports the [Aseprite extension](https://github.com/aseprite/aseprite/blob/main/docs/gpl-palette-extension.md) for alpha values\n");

	Log::printf("\n");
	Log::printf("## Images/textures\n");
	Log::printf("\n");
	Log::printf("| Name                        | Extension |\n");
	Log::printf("| :-------------------------- | --------- |\n");
	formatDescriptions.clear();
	for (const io::FormatDescription *desc = io::format::images(); desc->valid(); ++desc) {
		formatDescriptions.push_back(*desc);
	}
	formatDescriptions.sort(core::Greater<io::FormatDescription>());
	for (const io::FormatDescription &desc : formatDescriptions) {
		Log::printf("| %-27s | %-9s |\n", desc.name.c_str(), desc.exts[0].c_str());
	}
}

#define REG_PATH_THUMBEXT "ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}"
#define VOXTHUMB_CLSID "{CD1F0EA0-283C-4D90-A41D-DEBD9207D91F}"
void FormatPrinter::printInstallerWix() {
	Log::printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	Log::printf("<!--\n");
	Log::printf("	using the formatprinter and capturing the stdout to write it into this file can add BOMs under windows\n");
	Log::printf("	if you are getting weird errors about unclosed tokens, remove the BOM\n");
	Log::printf("-->\n");
	Log::printf("<CPackWiXPatch>\n");
	Log::printf("	<!--  Fragment ID is from: <your build dir>/_CPack_Packages/win64/WIX/files.wxs -->\n");
	Log::printf("	<CPackWiXFragment Id=\"CM_CP_voxelthumb.vengi_voxelthumb.dll\">\n");
	Log::printf("		<RegistryKey Root=\"HKCR\" Key=\"CLSID\\" VOXTHUMB_CLSID "\" ForceDeleteOnUninstall=\"yes\">\n");
	Log::printf("			<RegistryValue Type=\"string\" Value=\"Vengi thumbnailer\" />\n");
	Log::printf("			<RegistryKey Key=\"InprocServer32\">\n");
	Log::printf("				<RegistryValue Type=\"string\" Value=\"[#CM_FP_voxelthumb.vengi_voxelthumb.dll]\" />\n");
	Log::printf("				<RegistryValue Type=\"string\" Name=\"ThreadingModel\" Value=\"Both\" />\n");
	Log::printf("			</RegistryKey>\n");
	Log::printf("		</RegistryKey>\n");
	core::StringSet uniqueExtensions;
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		for (const core::String &e : desc->exts) {
			if (!uniqueExtensions.insert(e)) {
				continue;
			}
			Log::printf("		<RegistryKey Root=\"HKCR\" Key=\".%s\">\n", e.c_str());
			Log::printf("			<RegistryValue Key=\"" REG_PATH_THUMBEXT"\" Type=\"string\" Value=\"" VOXTHUMB_CLSID "\" />\n");
			Log::printf("		</RegistryKey>\n");
		}
	}
	uniqueExtensions.clear();
	Log::printf("	</CPackWiXFragment>\n");
	Log::printf("	<CPackWiXFragment Id=\"CM_CP_voxedit.voxedit.vengi_voxedit.exe\">\n");
	Log::printf("		<Environment Id=\"PATH\" Name=\"PATH\" Value=\"[INSTALL_ROOT]\" Permanent=\"yes\" Part=\"last\" Action=\"set\" System=\"yes\" />\n");
	Log::printf("\n");
	Log::printf("		<!-- Open With -->\n");
	Log::printf("		<RegistryValue Root=\"HKCR\" Key=\"Applications\\vengi_voxedit.exe\" Type=\"string\" Name=\"FriendlyAppName\" Value=\"Vengi Voxel Editor\"/>\n");
	Log::printf("		<RegistryValue Root=\"HKCR\" Key=\"Applications\\vengi_voxedit.exe\\shell\\open\\command\" Type=\"string\" Value=\"[#CM_FP_voxedit.voxedit.vengi_voxedit.exe] &quot;%%1&quot;\"/>\n");
	Log::printf("\n");
	Log::printf("		<!-- App Paths -->\n");
	Log::printf("		<RegistryKey Root=\"HKLM\" Key=\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vengi_voxedit.exe\">\n");
	Log::printf("			<RegistryValue Type=\"string\" Value=\"[#CM_FP_voxedit.voxedit.vengi_voxedit.exe]\" />\n");
	Log::printf("			<RegistryValue Type=\"string\" Name=\"Path\" Value=\"[INSTALL_ROOT]\" />\n");
	Log::printf("		</RegistryKey>\n");
	Log::printf("\n");
	Log::printf("		<!-- Default Programs Capabilities -->\n");
	Log::printf("		<RegistryKey Root=\"HKLM\" Key=\"SOFTWARE\\vengi-voxedit\\Capabilities\">\n");
	Log::printf("			<RegistryValue Type=\"string\" Name=\"ApplicationName\" Value=\"vengi-voxedit\" />\n");
	Log::printf("			<RegistryValue Type=\"string\" Name=\"ApplicationDescription\" Value=\"Vengi Voxel Editor\" />\n");
	Log::printf("		</RegistryKey>\n");
	Log::printf("\n");
	Log::printf("		<!-- Registered file extensions -->\n");
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		for (const core::String &e : desc->exts) {
			if (!uniqueExtensions.insert(e)) {
				continue;
			}
			const core::String &m = uniqueMimetype(*desc);
			Log::printf("		<RegistryValue Root=\"HKLM\" Key=\"Software\\vengi-voxedit\\Capabilities\\FileAssociations\" Name=\".%s\" Value=\"vengi-voxedit.%s\" Type=\"string\" />\n", e.c_str(), e.c_str());
			Log::printf("		<RegistryValue Root=\"HKCR\" Key=\"Applications\\vengi_voxedit.exe\\SupportedTypes\" Name=\".%s\" Value=\"\" Type=\"string\" />\n", e.c_str());
			Log::printf("		<RegistryValue Root=\"HKCR\" Key=\".%s\\OpenWithProgids\" Name=\"vengi-voxedit.%s\" Value=\"\" Type=\"string\" />\n", e.c_str(), e.c_str());
			Log::printf("		<ProgId Id=\"vengi-voxedit.%s\" Description=\"%s\" Icon=\"CM_FP_voxedit.voxedit.vengi_voxedit.exe\">\n", e.c_str(), desc->name.c_str());
			Log::printf("			<Extension Id=\"%s\" ContentType=\"%s\">\n", e.c_str(), m.c_str());
			Log::printf("				<Verb Id=\"open\" TargetFile=\"CM_FP_voxedit.voxedit.vengi_voxedit.exe\" Argument=\"&quot;%%1&quot;\" />\n");
			Log::printf("			</Extension>\n");
			Log::printf("		</ProgId>\n");
			Log::printf("\n");
		}
	}
	Log::printf("\n");
	Log::printf("		<!-- Default Programs entry -->\n");
	Log::printf("		<RegistryValue Root=\"HKLM\" Key=\"SOFTWARE\\RegisteredApplications\" Name=\"vengi-voxedit\" Value=\"SOFTWARE\\vengi-voxedit\\Capabilities\" Type=\"string\" />\n");
	Log::printf("	</CPackWiXFragment>\n");
	Log::printf("</CPackWiXPatch>\n");
}

void FormatPrinter::printApplicationPlist() {
	Log::printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	Log::printf("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
		   "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
	Log::printf("<plist version=\"1.0\">\n");
	Log::printf("  <dict>\n");
	Log::printf("    <key>CFBundleDevelopmentRegion</key>\n");
	Log::printf("    <string>en-US</string>\n");
	Log::printf("    <key>CFBundleExecutable</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_EXECUTABLE_NAME}</string>\n");
	Log::printf("    <key>CFBundleShortVersionString</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_SHORT_VERSION_STRING}</string>\n");
	Log::printf("    <key>CFBundleLongVersionString</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_LONG_VERSION_STRING}</string>\n");
	Log::printf("    <key>CFBundleIdentifier</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_GUI_IDENTIFIER}</string>\n");
	Log::printf("    <key>CFBundleInfoDictionaryVersion</key>\n");
	Log::printf("    <string>6.0</string>\n");
	Log::printf("    <key>CFBundleIconFile</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_ICON_FILE}</string>\n");
	Log::printf("    <key>NSHumanReadableCopyright</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_COPYRIGHT}</string>\n");
	Log::printf("    <key>CFBundleName</key>\n");
	Log::printf("    <string>${MACOSX_BUNDLE_BUNDLE_NAME}</string>\n");
	Log::printf("    <key>CFBundlePackageType</key>\n");
	Log::printf("    <string>APPL</string>\n");
	Log::printf("    <key>NSHighResolutionCapable</key>\n");
	Log::printf("    <true/>\n");
	Log::printf("    <key>NSRequiresAquaSystemAppearance</key>\n");
	Log::printf("    <false/>\n");
	Log::printf("    <key>CFBundleDocumentTypes</key>\n");
	Log::printf("    <array>\n");
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		const core::String &m = uniqueMimetype(*desc);
		Log::printf("      <dict>\n");
		Log::printf("        <key>CFBundleTypeName</key>\n");
		Log::printf("        <string>%s</string>\n", desc->name.c_str());
		Log::printf("        <key>CFBundleTypeIconFile</key>\n");
		Log::printf("        <string>icon.icns</string>\n");
		Log::printf("        <key>CFBundleTypeMIMETypes</key>\n");
		Log::printf("        <array>\n");
		Log::printf("          <string>%s</string>\n", m.c_str());
		Log::printf("        </array>\n");
		Log::printf("        <key>CFBundleTypeExtensions</key>\n");
		Log::printf("        <array>\n");
		for (const core::String &e : desc->exts) {
			Log::printf("          <string>%s</string>\n", e.c_str());
		}
		Log::printf("        </array>\n");
		Log::printf("        <key>CFBundleTypeRole</key>\n");
		Log::printf("        <string>Editor</string>\n");
		Log::printf("        <key>NSDocumentClass</key>\n");
		Log::printf("        <string>AppDocument</string>\n");
		Log::printf("      </dict>\n");
	}
	Log::printf("    </array>\n");
	Log::printf("  </dict>\n");
	Log::printf("</plist>\n");
}

void FormatPrinter::printMimeInfo() {
	_uniqueMimetypes.clear();
	Log::printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	Log::printf("<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n");
	for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
		const core::String &m = uniqueMimetype(*desc);
		Log::printf("\t<mime-type type=\"%s\">\n", m.c_str());
		Log::printf("\t\t<comment>%s</comment>\n", desc->name.c_str());
		for (const core::String &e : desc->exts) {
			Log::printf("\t\t<glob pattern=\"*.%s\"/>\n", e.c_str());
		}
		for (const core::String &e : desc->magics) {
			Log::printf("\t\t<magic priority=\"50\">\n");
			if (SDL_isprint(e.first())) {
				Log::printf("\t\t\t<match type=\"string\" offset=\"0\" value=\"%s\"/>\n", e.c_str());
			} else {
				for (size_t i = 0; i < e.size(); ++i) {
					Log::printf("\t\t\t<match type=\"byte\" offset=\"%i\" value=\"%i\"/>\n", (int)i, (int)e[i]);
				}
			}
			Log::printf("\t\t</magic>\n");
		}
		Log::printf("\t</mime-type>\n");
	}
	Log::printf("</mime-info>\n");
}

void FormatPrinter::printJson(bool palette, bool image, bool voxel) {
	Log::printf("{");
	if (palette) {
		Log::printf("\"palettes\": [");
		int i = 0;
		for (const io::FormatDescription *desc = palette::palettes(); desc->valid(); ++desc) {
			if (i != 0) {
				Log::printf(",");
			}
			Log::printf("{");
			Log::printf("\"name\": \"%s\",", desc->name.c_str());
			Log::printf("\"extensions\": [");
			printJsonStringArray(desc->exts);
			Log::printf("],\"magics\": [");
			printJsonMagicArray(desc->magics);
			Log::printf("]");
			Log::printf("}");
			++i;
		}
		Log::printf("]");
	}
	if (image) {
		if (palette) {
			Log::printf(",");
		}
		Log::printf("\"images\": [");
		int i = 0;
		for (const io::FormatDescription *desc = io::format::images(); desc->valid(); ++desc) {
			if (i != 0) {
				Log::printf(",");
			}
			Log::printf("{");
			Log::printf("\"name\": \"%s\",", desc->name.c_str());
			Log::printf("\"extensions\": [");
			printJsonStringArray(desc->exts);
			Log::printf("],\"magics\": [");
			printJsonMagicArray(desc->magics);
			Log::printf("]");
			Log::printf("}");
			++i;
		}
		Log::printf("]");
	}
	if (voxel) {
		if (palette || image) {
			Log::printf(",");
		}
		Log::printf("\"voxels\": [");
		int i = 0;
		for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
			if (i != 0) {
				Log::printf(",");
			}
			Log::printf("{");
			Log::printf("\"name\": \"%s\",", desc->name.c_str());
			Log::printf("\"extensions\": [");
			printJsonStringArray(desc->exts);
			Log::printf("],\"magics\": [");
			printJsonMagicArray(desc->magics);
			Log::printf("],");
			const core::String &m = uniqueMimetype(*desc);
			Log::printf("\"mimetype\": \"%s\",", m.c_str());
			if (voxelformat::isMeshFormat(*desc)) {
				Log::printf("\"mesh\": true,");
			}
			if (voxelformat::isAnimationSupported(*desc)) {
				Log::printf("\"animation\": true,");
			} else {
				Log::printf("\"animation\": false,");
			}
			if (voxelSaveSupported(*desc)) {
				Log::printf("\"save\": true");
			} else {
				Log::printf("\"save\": false");
			}
			Log::printf("}");
			++i;
		}
		Log::printf("]");
	}
	Log::printf("}\n");
}

int main(int argc, char *argv[]) {
	const io::FilesystemPtr &filesystem = core::make_shared<io::Filesystem>();
	const core::TimeProviderPtr &timeProvider = core::make_shared<core::TimeProvider>();
	FormatPrinter app(filesystem, timeProvider);
	return app.startMainLoop(argc, argv);
}
