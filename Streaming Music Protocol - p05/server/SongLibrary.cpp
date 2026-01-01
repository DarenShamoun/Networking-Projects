/**
 * File: SongLibrary.cpp
 *
 * Implementation file the for SongLibrary class.
 */

#include <iostream>
#include <filesystem>

#include "SongLibrary.h"

using std::map;
using std::cout;
using std::string;

namespace fs = std::filesystem;

std::optional<fs::path> SongLibrary::get_song_file_path(uint32_t song_id) const {
	if (song_files.contains(song_id)) {
		return song_files.at(song_id);
	}
	else {
		return std::nullopt;
	}
}

std::optional<fs::path> SongLibrary::get_info_file_path(uint32_t song_id) const {
	if (info_files.contains(song_id)) {
		return info_files.at(song_id);
	}
	else {
		return std::nullopt;
	}
}

std:: string SongLibrary::get_song_list() const{
	std::string result;

	for (const auto& [id, path] : song_files){
		result += std::to_string(id) + ":" + path.filename().string() + "\n";
	}
	// If there is a newline following, it will be removed
	if (!result.empty() && result.back()== '\n'){
		result.pop_back();
	}
	return result;
}

void SongLibrary::scan_files(string dir) {
	uint32_t current_id = 0;

	// Loop through all files in the directory
	for(fs::directory_iterator entry(dir); entry != fs::directory_iterator(); ++entry) {
		string filename = entry->path().filename().string();

		// See if the current file is an MP3 file
		if (entry->path().extension() == ".mp3") {
			cout << "(" << current_id << ") " << filename;
			song_files[current_id] = entry->path();

			// Check if there is an associated info file, associating that
			// with the id.
			fs::path info_file_path = entry->path();
			info_file_path = info_file_path.replace_extension(".mp3.info");

			if (fs::is_regular_file(info_file_path)) {
				cout << " (with info file)\n";
				info_files[current_id] = info_file_path;
			}
			else {
				cout << "\n";
			}
			current_id++;
		}
	}
}
