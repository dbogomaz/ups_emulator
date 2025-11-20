#ifndef UPS_MODEL_CONFIG_H
#define UPS_MODEL_CONFIG_H

#include <string>
#include <vector>

struct UpsOids {
    std::string modelNameOID;
    std::string inputVoltageOID;
    std::string inputFreqOID;
    std::string outputVoltageOID;
    std::string batteryStatusOID;
    std::string chargeRemainingOID;
    std::string batteryTempOID;
    std::string bypassStatusOID;
};

class UpsModelConfig {
public:
    std::string modelName;
    UpsOids oids;
    std::vector<int> bypassValues;

    bool load(const std::string& path, const std::string& modelName);


private:
    std::string getBinaryDir() const;
};



#endif  // UPS_MODEL_CONFIG_H