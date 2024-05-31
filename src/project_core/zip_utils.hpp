#ifndef ZIP_UTILS_HPP
#define ZIP_UTILS_HPP

#include <string>

bool createZipFromDirectory(const std::string &dir, const std::string &zipFileName);
void mergeFiles(const std::string &exe, const std::string &zip, const std::string &output);
bool extractEmbeddedZip(const std::string &exePath, const std::string &outputDir);
bool initPhysFS(const char *argv0);
bool mountZipFile(const char *archive);

#endif // ZIP_UTILS_HPP
