#include <Constellation.h>

ConstellationMapper::ConstellationMapper(std::vector<CF32> Points, const size_t& BufferSize): SyncBlock(BufferSize, 1, 1){
    m_name = "ConstellationMapper";
    LOG_DEBUG("Created Constellation Mapper");
    points = std::move(Points);
    size_log2 = (long long)log2(points.size());
    packed = 0x00;
}


size_t ConstellationMapper::work(const size_t& n_inputItems, std::vector<U8>&  input, std::vector<CF32>& output){
    size_t outputIdx = 0;
    size_t j = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        packed |= (input[i] & 0x01) << ((size_log2-1)- j++);
        if(j == size_log2){
            CF32 sym = output[outputIdx++] = points[packed];
            packed = 0x00;
            j = 0;
        }
    }
    
    return outputIdx;
}


ConstellationMapper::~ConstellationMapper(){
    LOG_DEBUG("Destroyed Constellation Mapper");
}

ConstellationDemapper::ConstellationDemapper(std::vector<CF32> Points, const size_t& BufferSize): SyncBlock(BufferSize, 1, 1){
    m_name = "ConstellationDemapper";
    LOG_DEBUG("Created Constellation Demapper");
    points = std::move(Points);
    size_log2 = (long long)log2(points.size());
    packed = 0x00;
}

size_t ConstellationDemapper::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<U8>& output){
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems-log2(points.size()); i+=log2(points.size()))
    {
        continue;
    }
    return outputIdx;
}

ConstellationDemapper::~ConstellationDemapper(){
    LOG_DEBUG("Destroyed Constellation Demapper");
}
