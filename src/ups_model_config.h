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

    // описание последней ошибки
    std::string lastError;

    // загрузка конфигурации из файла path (абсолютного или относительного)
    bool load(const std::string& path, const std::string& section);

private:
    // чтение фактического пути (если относительный — преобразуется в путь рядом с бинарником)
    std::string resolvePath(const std::string& path) const;

    // путь к каталогу бинарника
    std::string getBinaryDir() const;

    // проверка обязательных полей
    bool validate(const std::string& section);
};

#endif  // UPS_MODEL_CONFIG_H