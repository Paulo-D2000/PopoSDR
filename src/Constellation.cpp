#include <Constellation.h>

ConstellationMapper::ConstellationMapper(std::vector<CF32> Points, const size_t& BufferSize):
  SyncBlock(BufferSize, (long long)log2(Points.size()), 1),
  m_const_Obj(Points)
{
    LOG_DEBUG("Created Constellation Mapper");
    size_log2 = (long long)log2(m_const_Obj.points.size());
    packed = 0x00;
}

ConstellationMapper::ConstellationMapper(Constellation ConstellationObject, const size_t& BufferSize):
  SyncBlock(BufferSize, (long long)log2(ConstellationObject.points.size()), 1),
  m_const_Obj(ConstellationObject)
{
    m_name = "ConstellationMapper";
    LOG_DEBUG("Created Constellation Mapper");
    size_log2 = (long long)log2(m_const_Obj.points.size());
    packed = 0x00;
}


size_t ConstellationMapper::work(const size_t& n_inputItems, std::vector<U8>&  input, std::vector<CF32>& output){
    size_t outputIdx = 0;
    size_t j = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        packed = (packed << 1) + (input[i] & 0x01);
        j++;
        if(j == size_log2){
            CF32 sym = output[outputIdx++] = m_const_Obj.points[packed];
            packed = 0x00;
            j = 0;
        }
    }
    
    return outputIdx;
}


ConstellationMapper::~ConstellationMapper(){
    LOG_DEBUG("Destroyed Constellation Mapper");
}

ConstellationDemapper::ConstellationDemapper(std::vector<CF32> Points, const size_t& BufferSize):
  SyncBlock(BufferSize, 1, (long long)log2(Points.size())),
  m_const_Obj(Points)
{
    m_name = "ConstellationDemapper";
    LOG_DEBUG("Created Constellation Demapper");
    size_log2 = (long long)log2(m_const_Obj.points.size());
}

ConstellationDemapper::ConstellationDemapper(Constellation ConstellationObject, const size_t& BufferSize):
  SyncBlock(BufferSize, 1, (long long)log2(ConstellationObject.points.size())),
  m_const_Obj(ConstellationObject)
{
    m_name = "ConstellationDemapper";
    LOG_DEBUG("Created Constellation Demapper");
    size_log2 = (long long)log2(m_const_Obj.points.size());
}

size_t ConstellationDemapper::work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<U8>& output){
    size_t outputIdx = 0;
    for (size_t i = 0; i < n_inputItems; i++)
    {
        size_t sym_idx = 0;
        m_const_Obj.compute_distance(input[i], sym_idx);

        for (int j = size_log2-1; j>-1; j--)
        {
            output[outputIdx++] = ((sym_idx >> j) & 1);
        }
    }
    return outputIdx;
}

ConstellationDemapper::~ConstellationDemapper(){
    LOG_DEBUG("Destroyed Constellation Demapper");
}
