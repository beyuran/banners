#include "serializer.h"

bool writeMatBinary(std::ofstream& ofs, const BannerInfo& in_banner)
{
    if(!ofs.is_open()){
        return false;
    }
    cv::Mat in_mat = in_banner.image;

    // serialize image (cv::Mat)
    if(in_mat.empty()){
        int s = 0;
        ofs.write((const char*)(&s), sizeof(int));
        return true;
    }
    int type = in_mat.type();
    char continuous = in_mat.isContinuous()? 1 : 0;

    ofs.write((const char*)(&in_mat.rows), sizeof(int));
    ofs.write((const char*)(&in_mat.cols), sizeof(int));
    ofs.write((const char*)(&type), sizeof(int));
    ofs.write((const char*)(&continuous), sizeof(char));
    if (continuous)
        ofs.write((const char*)(in_mat.data), in_mat.elemSize() * in_mat.total());
    else
    {
        const unsigned int row_size = in_mat.cols * in_mat.elemSize();
        for (int i = 0; i < in_mat.rows; i++)
            ofs.write((const char*)(in_mat.ptr(i)), row_size);
    }

    // serialize name
    int name_len = in_banner.name.length();
    ofs.write((const char*)(&name_len), sizeof(int));
    ofs.write((const char*)(in_banner.name.toStdString().c_str()), name_len);

    // serialize location
    int loc_len = in_banner.location.length();
    ofs.write((const char*)(&loc_len), sizeof(int));
    ofs.write((const char*)(in_banner.location.toStdString().c_str()), loc_len);

    return true;
}

bool SaveMatBinary(const std::string& filename, const BannerInfo& in_banner){
    std::ofstream ofs(filename, std::ios::binary);
    return writeMatBinary(ofs, in_banner);
}

bool readMatBinary(std::ifstream& ifs, BannerInfo& out_banner)
{
    if(!ifs.is_open()){
        return false;
    }

    int rows, cols, type;
    ifs.read((char*)(&rows), sizeof(int));
    if(rows==0){
        return true;
    }
    ifs.read((char*)(&cols), sizeof(int));
    ifs.read((char*)(&type), sizeof(int));
    char continuous;
    ifs.read((char*)(&continuous), sizeof(char));

    cv::Mat out_mat;
    out_mat.create(rows, cols, type);
    if (continuous)
        ifs.read((char*)(out_mat.data), out_mat.elemSize() * out_mat.total());
    else
    {
        const unsigned int row_size = out_mat.cols * out_mat.elemSize();
        for (int i = 0; i < out_mat.rows; i++)
            ifs.read((char*)(out_mat.ptr(i)), row_size);
    }

    out_banner.image = out_mat.clone();

    int name_len;
    char buf[256];
    ifs.read((char*)(&name_len), sizeof(int));
    ifs.read((char*)(buf), name_len);
    std::string name(buf, name_len);

    int loc_len;
    char buf2[256];
    ifs.read((char*)(&loc_len), sizeof(int));
    ifs.read((char*)(buf2), loc_len);
    std::string location(buf2, loc_len);

    out_banner.name = QString(name.c_str());
    out_banner.location = QString(location.c_str());
}

bool LoadMatBinary(const std::string& filename, BannerInfo& out_banner){
    std::ifstream ifs(filename, std::ios::binary);
    return readMatBinary(ifs, out_banner);
}

Serializer::Serializer()
{
}

void Serializer::saveMats(std::vector<BannerInfo> &banners, std::string filename)
{
    std::ofstream ofs(filename, std::ios::binary);
    if(!ofs.is_open()){
        return;
    }
    int version = CURR_ARCH_VERSION;
    ofs.write((const char*)(&version), sizeof(int));
    int matNum = banners.size();
    ofs.write((const char*)(&matNum), sizeof(int));
    for (auto banner : banners)
    {
        writeMatBinary(ofs, banner);
    }
}

void Serializer::loadMats(std::vector<BannerInfo> &banners, std::string filename)
{
    std::ifstream ifs(filename, std::ios::binary);
    if(!ifs.is_open()){
        return;
    }
    banners.clear();
    int arch_version;
    ifs.read((char*)(&arch_version), sizeof(int));
    int matNum;
    ifs.read((char*)(&matNum), sizeof(int));
    for (int i = 0; i < matNum; i++)
    {
        BannerInfo banner;
        readMatBinary(ifs, banner);
        banners.push_back(banner);
    }
}
