#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include "structures.h"

#define CURR_ARCH_VERSION 1

class Serializer
{
    static int arch_version;
public:
    Serializer();
    void static saveMats(std::vector<BannerInfo>& banners, std::string filename);
    void static loadMats(std::vector<BannerInfo>& banners, std::string filename);
};

#endif // SERIALIZER_H
