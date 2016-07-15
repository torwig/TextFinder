#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config
{

public:
    static const int kMinNumberOfThreads {1};
    static const int kMaxNumberOfThreads {128};
    static const int kMinNumberOfUrls {1};
    static const int kMaxNumberOfUrls {9999};

    static Config& instance();

    void setNumberOfThreads(int n);
    int getNumberOfThreads() const;

    void setNumberOfUrls(int n);
    int getNumberOfUrls() const;

private:
    Config();
    Config(const Config& other) = delete;
    Config& operator=(const Config& other) = delete;

    int mNumberOfThreads {kMinNumberOfThreads};
    int mNumberOfURls {kMinNumberOfUrls};

};

#endif // CONFIG_H
