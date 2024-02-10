#pragma once

#include <vector>
#include <Types.h>
#include <Buffer.h>
#include <Logging.h>
#include <Block.h>


struct CICRate{
    size_t Interpolation=1;
    size_t Decimation=1;
};

class CICFilter: public SyncBlock<CF32>
{
public:
    CICFilter(const size_t& Order=1, const CICRate& rate={1,1}, const size_t& BufferSize=0);
    
    CF32 filter(const CF32& input);

    size_t work(const size_t& n_inputItems, std::vector<CF32>&  input, std::vector<CF32>& output);

    ~CICFilter();

private:
    struct Integrator{
        CI64 value;
        CI64 update(CI64 input){
            value = value + input;
            return value;
        }
    };
    
    struct Comb{
        std::vector<CI64> delay;
        size_t ptr = 0;
        CI64 update(CI64 input){
            CI64 out = input - delay[ptr];
            delay[ptr++] = input;
            if(ptr == delay.size()){
                ptr = 0;
            }
            return out;
        }
    };

    CICRate m_rate;
    std::vector<Integrator> m_integ;
    std::vector<Comb> m_comb;
};
