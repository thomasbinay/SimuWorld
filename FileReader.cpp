#include "FileReader.hpp"
#include <fstream>
#include <SDL2/SDL.h>

void FileReader::read(std::string const& file_name)
{
    m_file_name = file_name;
    m_content.clear();
    std::ifstream file(file_name);
    if(file)
    {
        std::string buffer;
        while(getline(file, buffer))
        {
            m_content[buffer.substr(0, buffer.find('='))] = buffer.substr(buffer.find('=') + 1);
        }
        file.close();
    }
    else
    {
        std::string title = m_file_name;
        std::string body = "le fichier n'a pas pu etre ouvert";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
}

std::string FileReader::getString(std::string const& data) const
{
    try
    {
        return m_content.at(data);
    }
    catch (std::out_of_range const& oor) 
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" est manquante";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
}

int FileReader::getInt(std::string const& data) const
{
    try
    {
        return std::stoi(m_content.at(data));
    }
    catch (std::out_of_range const& oor) 
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" est manquante";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
    catch(std::invalid_argument const& ia)
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" n'est pas un entier";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
}

double FileReader::getDouble(std::string const& data) const
{
    try
    {
        return std::stod(m_content.at(data));
    }
    catch (std::out_of_range const& oor) 
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" est manquante";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
    catch(std::invalid_argument const& ia)
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" n'est pas un nombre decimal";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
}

std::vector<int> FileReader::getVectorInt(std::string const& data) const
{
    std::string buffer;
    try
    {
        buffer = m_content.at(data);
    }
    catch (std::out_of_range const& oor) 
    {
        std::string title = m_file_name;
        std::string body = "la valeur \"" + data + "\" est manquante";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), body.c_str(), NULL);
        exit(0);
    }
    std::vector <int> values;
    while(buffer.find(',') != std::string::npos)
    {
        values.push_back(std::stoi(buffer.substr(0, buffer.find(','))));
        buffer = buffer.substr(buffer.find(',') + 1);
    }
    return values;
}

int FileReader::getCount() const
{
    return m_content.size();
}