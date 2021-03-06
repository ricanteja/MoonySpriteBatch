#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H


#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <SFML/Graphics.hpp>

#ifdef USE_ZLIB
	#include <zlib.h>
#endif


#include "Log.h"
#include "Texture.h"


namespace moony
{
	// The TextureAtlas class holds all the Moony Texture objects inside Atlas objects which can be 
	// referenced by the name of the original file they were loaded as. Only one of these objects should be necessary.
	class TextureAtlas
	{
	public:
		bool loadFromFile(const std::string& file_path);
		const Texture findSubTexture(std::string name);
		std::vector<std::string> getSubTextureNames();

	private:
		struct Atlas
		{
			Atlas() : m_texture(new sf::Texture) {};
			std::unique_ptr<sf::Texture> m_texture;
			std::unordered_map<std::string, sf::IntRect> m_texture_table;
		};

		std::vector<Atlas> m_atlas_list;
	};

	// Loads a Moony Texture Pack File (.mtpf) to populate the atlas table. These files are generated by the Moony Texture Packer tool.
	// Returns false if failed to load the file, otherwise returns true.
	bool TextureAtlas::loadFromFile(const std::string& file_path)
	{
		std::ifstream input_file(file_path, std::ifstream::in | std::ifstream::binary);

		if(!input_file)
			return false;

		std::string path = file_path.substr(0, file_path.find_last_of("\\/") + 1);
		std::string input;

		while(input_file >> input)
		{
			m_atlas_list.push_back(Atlas());

			if(input == "D")
			{
#ifdef USE_ZLIB
				sf::Vector2u img_size;
				unsigned long comp_size;

				(input_file >> img_size.x >> img_size.y >> comp_size).ignore(1);

				unsigned long data_size = img_size.x * img_size.y * 4;
				char* data = new char[data_size];

				input.resize(comp_size);
				input_file.read(&input[0], comp_size);
				uncompress((Bytef*)data, &data_size, (Bytef*)&input[0], comp_size);

				m_atlas_list.back().m_texture->create(img_size.x, img_size.y);
				m_atlas_list.back().m_texture->update((sf::Uint8*)data);

				delete data;
#endif
			}
			else
			{
				size_t name_len;
				(input_file >> name_len).ignore(1);
				input.resize(name_len);
				input_file.read(&input[0], name_len);

				m_atlas_list.back().m_texture->loadFromFile(path + input);
			}

			size_t img_count;
			input_file >> img_count;

			for(size_t index = 0; index < img_count; index++)
			{
				size_t name_len;
				(input_file >> name_len).ignore(1);
				input.resize(name_len);
				input_file.read(&input[0], name_len);

				sf::IntRect texture_rect;
				input_file >> texture_rect.left >> texture_rect.top >> texture_rect.width >> texture_rect.height;

				m_atlas_list.back().m_texture_table[input] = texture_rect;
			}
		}

		input_file.close();

		if(!m_atlas_list.size())
			return false;

		return true;
	}

	// Finds a Moony Texture inside the atlas table.
	// Returns a valid Texture if found in the TextureAtlas, otherwise returns a null Texture.
	const Texture TextureAtlas::findSubTexture(std::string name)
	{
		Texture texture;

		for(auto& atlas : m_atlas_list)
		{
			if(atlas.m_texture_table.count(name) > 0)
			{
				texture = Texture(atlas.m_texture.get(), atlas.m_texture_table[name]);
				break;
			}
		}

		return texture;
	}

	// This function gets a list of all the seperate image names that are in the TextureAtlas.
	// Returns a std::vector<std::string> full of filenames corresponding to images in the TextureAtlas.
	std::vector<std::string> TextureAtlas::getSubTextureNames()
	{
		std::vector<std::string> names;

		for(const auto& atlas : m_atlas_list)
		{
			for(const auto& itr : atlas.m_texture_table)
				names.push_back(itr.first);
		}

		return names;
	}

}

#endif