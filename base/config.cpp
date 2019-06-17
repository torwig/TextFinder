#include "config.h"

Config::Config()
{

}

Config& Config::instance()
{
    static Config config;
    return config;
}

void Config::setNumberOfThreads(int n)
{
    mNumberOfThreads = n;
}

int Config::getNumberOfThreads() const
{
    return mNumberOfThreads;
}

void Config::setNumberOfUrls(int n)
{
    mNumberOfURls = n;
}

int Config::getNumberOfUrls() const
{
    return mNumberOfURls;
}
