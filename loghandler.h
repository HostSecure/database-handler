#ifndef LOGHANDLER_H
#define LOGHANDLER_H

class QFile;

class LogHandler
{
public:
    friend int main(int argc, char *argv[]);

private:
    LogHandler();
    ~LogHandler();
};

#endif // LOGHANDLER_H
