#include "FileReader.h"

namespace FileReader {

    namespace {
        const char* currentFile;

        std::string getFileName(std::string& path)
        {
            // Find the position of the last path separator ('/' or '\\')
            size_t pos = path.find_last_of("/\\");

            // If found, extract the substring after the last separator, otherwise use the whole path
            if (pos != std::string::npos) {
                return path.substr(pos + 1); // Substring from position after the last separator
            }

            return path;
        }
    }

    const char* getCurrentFile()
    {
        return currentFile;
    }

	std::vector<Column> readCSV(const char* filePath)
    {
        std::vector<Column> columns;
        std::ifstream file(filePath);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open CSV file " << filePath << std::endl;
            return columns;
        }

        std::string line;

        // Read the header (first line with column names)
        if (std::getline(file, line)) {
            std::stringstream headerStream(line);
            std::string columnName;

            // Initialize columns with names
            while (std::getline(headerStream, columnName, ',')) {
                columns.push_back({ columnName, {} });
            }
        }

        // Read the data
        unsigned int rowIndex = 0;
        while (std::getline(file, line)) {
            std::stringstream lineStream(line);
            std::string value;
            unsigned int columnIndex = 0;

            // Parse each value in the line and add it to the corresponding column
            while (std::getline(lineStream, value, ',')) {
                if (columnIndex < columns.size()) {
                    try {
                        columns[columnIndex].data.push_back(std::stof(value));
                    } catch (std::exception e) {
                        std::cout << "Error reading CSV file: " << e.what() << std::endl;
                    }
                }
                columnIndex++;
            }

            if (columnIndex != columns.size())
            {
                std::cout << "Error: CSV file contains too few columns on row " << rowIndex << std::endl;
            }

            rowIndex++;
        }

        file.close();

        std::string pathStr = std::string(filePath);
        std::string fileName = getFileName(pathStr);

        std::cout << "Read CSV file " << fileName << " with columns ";
        unsigned int columnIndex = 0;
        for (const Column& column : columns)
        {
            std::cout << column.name;

            bool lastColumn = columnIndex == (columns.size() - 1);
            if (!lastColumn)
            {
                std::cout << ", ";
            }

            columnIndex++;
        }
        std::cout << " and " << columns[0].data.size() << " rows." << std::endl;

        currentFile = filePath;
        SavedSettings::setPreviouslyLoadedFile(std::string(filePath));

        return columns;
    }

    std::vector<int> readLabels(const char* filePath, unsigned int sampleFrequency)
    {
        std::vector<int> labels;
        std::unordered_map<std::string, unsigned int> labelMap;

        std::ifstream file(filePath);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open CSV file " << filePath << std::endl;
            return labels;
        }

        std::string line;
        unsigned int index = 0;
        while (std::getline(file, line)) {
            std::stringstream lineStream(line);

            if (index % sampleFrequency != 0) {
                index++;
                continue;
            }

            // Check if the value is already in the map
            if (labelMap.find(line) == labelMap.end()) {
                // Assign a new integer to this unique string
                labelMap[line] = static_cast<unsigned int>(labelMap.size()); // labelMap.size() gives the next integer (0, 1, 2, ...)
            }

            // Add the integer label to the result vector
            labels.push_back(labelMap[line]);

            index++;
        }

        file.close();

        return labels;
    }
};