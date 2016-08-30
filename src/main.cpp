#include "command.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    buildCommandLineParser(&parser, app);
    return runCommand(parser, app);
}
