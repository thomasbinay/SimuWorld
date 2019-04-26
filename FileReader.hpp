#ifndef DEF_FILEREADER
#define DEF_FILEREADER

#include <map>
#include <vector>


class FileReader
{
public:
    void read(std::string const& file_name);
    std::string getString(std::string const& data) const;
    int getInt(std::string const& data) const;
    double getDouble(std::string const& data) const;
    std::vector<int> getVectorInt(std::string const& data) const;
    int getCount() const;

private:
    std::map <std::string, std::string> m_content;
    std::string m_file_name;
};

#endif