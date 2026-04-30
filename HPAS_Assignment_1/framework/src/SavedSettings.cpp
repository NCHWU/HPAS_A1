#include "SavedSettings.h"

namespace SavedSettings 
{
    namespace
    {
        const std::string SAVE_DIR = "saved/";
        const std::string PREVIOUS_FILE_PATH = "previous_dataset.txt";

        // Function to save data to a file
        void save(std::string fileName, const char* data)
        {
            // Ensure the directory exists
            std::filesystem::create_directories(SAVE_DIR);

            // Open file for writing
            std::ofstream file(SAVE_DIR + fileName);
            if (file.is_open()) {
                file << data;
                file.close();
                std::cout << "Data saved to " << SAVE_DIR + fileName << std::endl;
            }
            else {
                std::cerr << "Failed to open file for writing: " << SAVE_DIR + fileName << std::endl;
            }
        }

        // Function to read string data from a file
        std::string readString(std::string fileName)
        {
            std::ifstream file(SAVE_DIR + fileName);
            std::string content;

            if (file.is_open()) {
                std::getline(file, content, '\0'); // Read entire file content
                file.close();
            }
            else {
                std::cerr << "Failed to open file for reading: " << SAVE_DIR + fileName << std::endl;
            }

            return content;
        }
    }

    // Function to save the path of the previously loaded file
    void setPreviouslyLoadedFile(std::string path)
    {
        save(PREVIOUS_FILE_PATH, path.c_str());
    }

    // Function to get the path of the previously loaded file
    std::string getPreviouslyLoadedFile()
    {
        return readString(PREVIOUS_FILE_PATH);
    }
}
