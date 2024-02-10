#include <Block.h>
#include <vector>

class ConstellationMapper: public SyncBlock<U8, CF32>
{
private:
    U8 packed;
    size_t size_log2;
    std::vector<CF32> points;
public:
    ConstellationMapper(std::vector<CF32> Points, const size_t& BufferSize);

    size_t work(const size_t& n_inputItems, std::vector<U8>&  input, std::vector<CF32>& output);

    ~ConstellationMapper();
};

class ConstellationDemapper: public SyncBlock<CF32, U8>
{
private:
    U8 packed;
    size_t size_log2;
    std::vector<CF32> points;
public:
    ConstellationDemapper(std::vector<CF32> Points, const size_t& BufferSize=0);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<U8>& output);

    ~ConstellationDemapper();
};