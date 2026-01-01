#ifndef __SONGLIBRARY_H__
#define __SONGLIBRARY_H__
/**
 * File: SongLibrary.h
 *
 * Header file the for SongLibrary class.
 */

#include <string>
#include <map>
#include <optional>
#include <filesystem>

/**
 * Class that represents the library of songs available.
 */
class SongLibrary {
  private:
	std::map<uint32_t, std::filesystem::path> song_files;
	std::map<uint32_t, std::filesystem::path> info_files;

  public:
	/**
	 * Returns the path to the song file with the given song id, if it exists.
	 *
	 * @param song_id
	 */
	std::optional<std::filesystem::path> get_song_file_path(uint32_t song_id) const;

	/**
	 * Returns the path to the info file with the given song id, if it exists.
	 *
	 * @param song_id
	 */
	std::optional<std::filesystem::path> get_info_file_path(uint32_t song_id) const;

	/**
	 * Returns the number of songs in the library.
	 */
	uint32_t num_songs() const { return song_files.size(); }

	/**
	 * Search the given directory for any files that end in ".mp3", assigning
	 * them a unique ID and mapping the ID to the path to the file in
	 * this->song_files.
	 * When it files an MP3 file, it also looks for an associated ".info"
	 * file, mapping the ID of the song with the path to the info file in
	 * this->info_files.
	 *
	 * @param dir String that represents the path to the directory that you want
	 * 				to check.
	 */
	void scan_files(std::string dir);

	/**
	 * Returns a formatted string that lists all available songs
	 */
	std::string get_song_list() const;
};

#endif
