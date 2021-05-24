#include "configuration.hpp"

#include <unordered_map>

/// Maps names to configuration objects
std::unordered_map<std::string, Configuration> configs;

void setConfiguration(const std::string &name, Configuration config)
{
    //ATENTIE!! Numele este unic, daca se primesc alte setari cu acelasi nume se considera update.
    configs.insert_or_assign(name, config);
}

const Configuration &getConfiguration(const std::string &name)
{
    return configs.at(name);
}

std::string getProgramName(ProgramType program)
{
    switch (program)
    {
    case ProgramType::Full_Clean:
        return "Full_Clean";
    case ProgramType::Only_Upper:
        return "Only Upper";
    case ProgramType::Only_Lower:
        return "Only Lower";
    case ProgramType::Warning_Safe_Teeths_Full_Clean:
        return "Warning Safe Teeth Full Clean";
    default:
        return "Unknown Config";
    }
}
