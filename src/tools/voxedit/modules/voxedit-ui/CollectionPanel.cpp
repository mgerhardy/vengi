/**
 * @file
 */

#include "CollectionPanel.h"
#include "IconsLucide.h"
#include "command/CommandHandler.h"
#include "core/StringUtil.h"
#include "ui/IMGUIEx.h"
#include "ui/IMGUIApp.h"
#include "video/gl/GLTypes.h"
#include "voxedit-util/SceneManager.h"
#include "voxedit-util/modifier/Modifier.h"
#include "voxelcollection/Downloader.h"
#include "voxelformat/VolumeFormat.h"

namespace voxedit {

CollectionPanel::CollectionPanel(ui::IMGUIApp *app, const SceneManagerPtr &sceneMgr,
								 const voxelcollection::CollectionManagerPtr &collectionMgr,
								 const video::TexturePoolPtr &texturePool)
	: Super(app, "collection"), _sceneMgr(sceneMgr), _collectionMgr(collectionMgr), _texturePool(texturePool) {
}

CollectionPanel::~CollectionPanel() {
}

bool CollectionPanel::filtered(const voxelcollection::VoxelFile &voxelFile) const {
	if (!_currentFilterName.empty() && !core::string::icontains(voxelFile.name, _currentFilterName)) {
		return true;
	}
	if (!_currentFilterLicense.empty() && !core::string::icontains(voxelFile.license, _currentFilterLicense)) {
		return true;
	}
	if (_currentFilterFormatEntry <= 0) {
		return false;
	}
	const core::String &filter = _filterEntries[_currentFilterFormatEntry].wildCard();
	if (core::string::fileMatchesMultiple(voxelFile.name.c_str(), filter.c_str())) {
		return false;
	}
	return true;
}

bool CollectionPanel::isFilterActive() const {
	return !_currentFilterName.empty() || !_currentFilterLicense.empty() || _currentFilterFormatEntry > 0;
}

void CollectionPanel::updateFilters() {
	const float itemWidth = ImGui::CalcTextSize("#").x * 9.0;
	{
		ImGui::PushItemWidth(itemWidth);
		ImGui::InputText(_("Name"), &_currentFilterName);
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}
	{
		ImGui::PushItemWidth(itemWidth);
		ImGui::InputText(_("License"), &_currentFilterLicense);
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}
	{
		if (_filterFormatTextWidth < 0.0f) {
			for (const io::FormatDescription *desc = voxelformat::voxelLoad(); desc->valid(); ++desc) {
				_filterEntries.push_back(*desc);
				const core::String &str = io::convertToFilePattern(*desc);
				const ImVec2 filterTextSize = ImGui::CalcTextSize(str.c_str());
				_filterFormatTextWidth = core_max(_filterFormatTextWidth, filterTextSize.x);
			}
			_filterEntries.sort(core::Greater<io::FormatDescription>());
			io::createGroupPatterns(voxelformat::voxelLoad(), _filterEntries);
			// must be the first entry - see applyFilter()
			_filterEntries.insert(_filterEntries.begin(), io::ALL_SUPPORTED());
			_filterFormatTextWidth = core_min(itemWidth * 2.0f, _filterFormatTextWidth);
		}

		const char *formatFilterLabel = _("Format");
		ImGui::PushItemWidth(_filterFormatTextWidth);
		int currentlySelected = _currentFilterFormatEntry == -1 ? 0 : _currentFilterFormatEntry;
		const core::String &selectedEntry = io::convertToFilePattern(_filterEntries[currentlySelected]);

		if (ImGui::BeginCombo(formatFilterLabel, selectedEntry.c_str(), ImGuiComboFlags_HeightLargest)) {
			for (int i = 0; i < (int)_filterEntries.size(); ++i) {
				const bool selected = i == currentlySelected;
				const io::FormatDescription &format = _filterEntries[i];
				const core::String &text = io::convertToFilePattern(format);
				if (ImGui::Selectable(text.c_str(), selected)) {
					_currentFilterFormatEntry = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
	}
}

int CollectionPanel::update() {
	core_trace_scoped(CollectionPanel);
	int cnt = 0;
	const voxelcollection::VoxelFileMap &voxelFilesMap = _collectionMgr->voxelFilesMap();
	updateFilters();

	if (ImGui::IconButton(ICON_LC_FOLDER, _("Local directory"))) {
		_app->directoryDialog([&] (const core::String &folderName, const io::FormatDescription *desc) {
			_collectionMgr->setLocalDir(folderName);
		}, {});
	}
	ImGui::TooltipTextUnformatted(_collectionMgr->localDir().c_str());

	const int columns = _thumbnails ? 3 : 2;
	if (ImGui::BeginTable("##voxelfiles", columns,
						  ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders |
							  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupScrollFreeze(0, 1);
		if (_thumbnails) {
			ImGui::TableSetupColumn(_("Thumbnail"));
		}
		ImGui::TableSetupColumn(_("Name"));
		ImGui::TableSetupColumn(_("License"));
		ImGui::TableHeadersRow();
		for (const auto &source : _collectionMgr->sources()) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_SpanAllColumns |
										   ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			auto iter = voxelFilesMap.find(source.name);
			if (iter != voxelFilesMap.end()) {
				const voxelcollection::VoxelCollection &collection = iter->second;
				if (isFilterActive() && _collectionMgr->resolved(source)) {
					treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				} else if (!collection.sorted) {
					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				}
				const int n = (int)collection.files.size();
				const core::String &label = core::string::format("%s (%i)##%s", source.name.c_str(), n, source.name.c_str());
				if (ImGui::TreeNodeEx(label.c_str(), treeFlags)) {
					if (!collection.sorted) {
						ImGui::TextUnformatted(_("Loading..."));
					} else {
						const voxelcollection::VoxelFiles &voxelFiles = collection.files;
						cnt += buildVoxelTree(voxelFiles);
					}
					ImGui::TreePop();
				}
				if (source.isLocal()) {
					ImGui::TooltipText("%s", _collectionMgr->localDir().c_str());
				}
			} else {
				if (!_collectionMgr->resolved(source)) {
					treeFlags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Bullet;
				}
				if (ImGui::TreeNodeEx(source.name.c_str(), treeFlags)) {
					// if resolved already but no files are available, we are still loading...
					if (_collectionMgr->resolved(source)) {
						ImGui::TextUnformatted(_("Loading..."));
					} else {
						_collectionMgr->resolve(source);
					}
					ImGui::TreePop();
				} else {
					ImGui::TooltipTextUnformatted(_("Double click to load"));
				}
			}
		}
		ImGui::EndTable();
	}
	return cnt;
}

void CollectionPanel::contextMenu(voxelcollection::VoxelFile *voxelFile) {
	if (ImGui::BeginPopupContextItem()) {
		if (!voxelFile->downloaded) {
			_collectionMgr->download(*voxelFile);
		}

		if (ImGui::MenuItem(_("Use stamp"))) {
			Modifier &modifier = _sceneMgr->modifier();
			StampBrush &brush = modifier.stampBrush();
			if (brush.load(voxelFile->targetFile())) {
				modifier.setBrushType(BrushType::Stamp);
			} else {
				Log::error("Failed to load stamp brush");
			}
		}
		ImGui::TooltipTextUnformatted(
			_("This is only possible if the model doesn't exceed the max allowed stamp size"));

		if (ImGui::MenuItem(_("Add to scene"))) {
			import(voxelFile);
		}

		if (_thumbnails) {
			if (ImGui::MenuItem(_("Hide thumbnails"))) {
				_thumbnails = false;
			}
		} else {
			if (ImGui::MenuItem(_("Show thumbnails"))) {
				_thumbnails = true;
			}
		}

		if (!io::isA(voxelFile->name, voxelformat::voxelLoad())) {
			if (ImGui::MenuItem(_("Open target file"))) {
				const core::Path &absPath = _collectionMgr->absolutePath(*voxelFile);
				command::executeCommands("url \"file://" + absPath.str() + "\"");
			}
			if (ImGui::MenuItem(_("Open target dir"))) {
				const core::Path &absPath = _collectionMgr->absolutePath(*voxelFile);
				const core::Path &dir = absPath.dirname();
				command::executeCommands("url \"file://" + dir.str() + "\"");
			}
		} else if (!thumbnailLookup(*voxelFile)) {
			if (ImGui::MenuItem(_("Create thumbnail"))) {
				_collectionMgr->createThumbnail(*voxelFile);
			}
		}

		ImGui::EndPopup();
	}
}

bool CollectionPanel::import(voxelcollection::VoxelFile *voxelFile) {
	if (!voxelFile->downloaded) {
		_collectionMgr->download(*voxelFile);
	}
	if (!voxelFile->downloaded) {
		return false;
	}
	Log::debug("Try to import %s", voxelFile->targetFile().c_str());
	return _sceneMgr->import(voxelFile->targetFile());
}

void CollectionPanel::handleDoubleClick(voxelcollection::VoxelFile *voxelFile) {
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		import(voxelFile);
		_selected = *voxelFile;
	}
}

void CollectionPanel::thumbnailTooltip(voxelcollection::VoxelFile *&voxelFile) {
	if (const video::TexturePtr &texture = thumbnailLookup(*voxelFile)) {
		if (ImGui::BeginItemTooltip()) {
			const video::Id handle = texture->handle();
			ImGui::Image(handle, ImGui::Size(40.0f));
			ImGui::Text("%s", voxelFile->fullPath.c_str());
			ImGui::EndTooltip();
		}
	}
}
int CollectionPanel::buildVoxelTree(const voxelcollection::VoxelFiles &voxelFiles) {
	core::DynamicArray<voxelcollection::VoxelFile *> f;
	f.reserve(voxelFiles.size());

	for (voxelcollection::VoxelFile &voxelFile : voxelFiles) {
		if (filtered(voxelFile)) {
			continue;
		}
		f.push_back(&voxelFile);
	}
	if (f.empty()) {
		return 0;
	}

	ImGuiListClipper clipper;
	clipper.Begin((int)f.size());

	while (clipper.Step()) {
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
			voxelcollection::VoxelFile *voxelFile = f[row];

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			const bool selected = _selected == *voxelFile;

			if (_thumbnails) {
				video::Id handle;
				if (const video::TexturePtr &texture = thumbnailLookup(*voxelFile)) {
					handle = texture->handle();
				} else {
					handle = video::InvalidId;
				}
				core::String id = core::string::format("%i", row);
				// TODO: dpi awareness
				if (ImGui::ImageButton(id.c_str(), handle, ImVec2(64, 64))) {
					if (!voxelFile->downloaded) {
						_collectionMgr->download(*voxelFile);
					}
					if (handle == video::InvalidId) {
						_collectionMgr->createThumbnail(*voxelFile);
					}
				}
				if (handle == video::InvalidId) {
					if (ImGui::BeginItemTooltip()) {
						ImGui::TextUnformatted(_("Double click to create thumbnail"));
						ImGui::Text("%s", voxelFile->fullPath.c_str());
						ImGui::EndTooltip();
					}
				} else {
					thumbnailTooltip(voxelFile);
				}
				ImGui::TableNextColumn();
			}
			if (ImGui::Selectable(voxelFile->name.c_str(), selected,
								  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
				handleDoubleClick(voxelFile);
			}
			if (!_thumbnails) {
				thumbnailTooltip(voxelFile);
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}

			contextMenu(voxelFile);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(voxelFile->license.c_str());
		}
	}

	if (f.empty() && isFilterActive()) {
		ImGui::TextUnformatted(_("No match for filter found"));
	}
	return (int)f.size();
}

video::TexturePtr CollectionPanel::thumbnailLookup(const voxelcollection::VoxelFile &voxelFile) {
	static video::TexturePtr empty;
	if (_texturePool->has(voxelFile.name)) {
		return _texturePool->get(voxelFile.name);
	}
	return empty;
}

void CollectionPanel::shutdown() {
	_filterEntries.clear();
}

bool CollectionPanel::init() {
	return true;
}

voxelcollection::VoxelFile &CollectionPanel::selected() {
	return _selected;
}

} // namespace voxedit
