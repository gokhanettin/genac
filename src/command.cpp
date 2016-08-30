#include "command.h"
// #include "config.h"
#include "Analyzer/circuit.h"
#include "Analyzer/netlist.h"
#include "Analyzer/analyzer.h"
#include "Analyzer/pretty.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

static int analyze(const QString& netlistFile, const QString& libraryDirectory)
{
    QTextStream console(stdout);
    Netlist netlist;
    QString err;
    netlist.parse(&err, netlistFile, libraryDirectory);
    if(!err.isEmpty()) {
        console << "Netlist Parser Error: " << err << "\n";
        console.flush();
        return -1;
    }
    Circuit* cir = netlist.circuit();
    Analyzer analyzer(cir);
    analyzer.solve();
    delete cir;
    QList<Analyzer::TransferFunction> t_funcs = analyzer.calcTFs(netlist.tfs());
    QString output;
    for(int i = 0; i < t_funcs.size(); ++i){
        output += pretty(t_funcs.at(i));
        output += "\n";
    }
    console << output;
    console.flush();
    return 0;
}

static int synthesize(const QString& outfile, const QString& nmask,
                      const QString& dmask, const QString& active,
                      const QString& xover, int rc, int cc, bool pcost,
                      bool gcost, bool adaptive)
{
    return 0;
}

void buildCommandLineParser(QCommandLineParser* parser, const QCoreApplication& app)
{
    // QCoreApplication::setApplicationName(GENAC);
    // QCoreApplication::setApplicationVersion(GENAC_VERSION);
    // parser->setApplicationDescription(GENAC_DESCRIPTION);
    parser->addHelpOption();
    parser->addVersionOption();

    parser->addPositionalArgument("command",
                    QCoreApplication::translate("main", "analyze | synthesize"));

    parser->parse(QCoreApplication::arguments());
    const QStringList args = parser->positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();
    if (command == "synthesize") {
        parser->clearPositionalArguments();
        parser->addPositionalArgument("synthesize",
                QCoreApplication::translate("main", "Generate a circuit which meets given requirements"));
        parser->addOptions({
            {{"o", "output-file"},
                QCoreApplication::translate("main", "Write solutions to <output-file>."),
                QCoreApplication::translate("main", "output-file")},
            {{"n", "numerator"},
                QCoreApplication::translate("main", "Search for binary numerator <mask> (e.g. 010) Should match denominator in size."),
                QCoreApplication::translate("main", "mask")},
            {{"d", "denominator"},
                QCoreApplication::translate("main", "Search for binary denominator <mask> (e.g. 111) Should match numerator in size."),
                QCoreApplication::translate("main", "mask")},
            {{"r", "nresistor"},
                QCoreApplication::translate("main", "Specify the number of resistors."),
                QCoreApplication::translate("main", "number")},
            {{"c", "ncapacitor"},
                QCoreApplication::translate("main", "Specify the number of capacitors."),
                QCoreApplication::translate("main", "number")},
            {{"a", "active-element"},
                    QCoreApplication::translate("main", "Use given active element <type> OPAMP | OTRA."),
                QCoreApplication::translate("main", "type")},
            {{"x", "crossover"},
                QCoreApplication::translate("main", "Perform crossover with a given <type> ONEPOINT | TWOPOINT | UNIFORM | SINGLEGENE."),
                QCoreApplication::translate("main", "type")},
            {"p",
                QCoreApplication::translate("main", "Use phenotype diversity cost.")},
            {"g",
                QCoreApplication::translate("main", "Use genotype diversity cost.")},
            {"m",
                QCoreApplication::translate("main", "Use adaptive mutation probability.")},
            });
        parser->process(app);
    } else if (command == "analyze") {
        parser->clearPositionalArguments();
        parser->addPositionalArgument("analyze",
        QCoreApplication::translate("main", "Perform a symbolic circuit analysis on a given netlist"));

        parser->addOptions({
            {{"n", "netlist-file"},
                QCoreApplication::translate("main", "Analyze circuit given in input <netlist-file>."),
                QCoreApplication::translate("main", "netlist-file")},
            {{"l", "library-directory"},
                QCoreApplication::translate("main", "Use library entries in <directory>."),
                QCoreApplication::translate("main", "directory")},
            });
        parser->process(app);
    }
}

int runCommand(QCommandLineParser& parser, QCoreApplication& app)
{
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(-1);
    }
    if (args[0] == "analyze") {
        const QString netlistFile = parser.value("netlist-file");
        const QString libraryDirectory = parser.value("library-directory");
        if (netlistFile.isEmpty()) {
            parser.showHelp(-1);
        }
        return analyze(netlistFile, libraryDirectory);
    } else if (args[0] == "synthesize") {
        bool ok = false;
        const QString outfile = parser.value("output-file");
        const QString nmask = parser.value("numerator");
        const QString dmask = parser.value("denominator");
        QString active = parser.value("active-element");
        QString xover = parser.value("crossover");
        int rc = parser.value("nresistor").toInt(&ok);
        if (!ok || rc <= 0) {
            parser.showHelp(-1);
        }
        int cc = parser.value("ncapacitor").toInt(&ok);
        if (!ok || cc <= 0) {
            parser.showHelp(-1);
        }
        if (outfile.isEmpty() || nmask.isEmpty() || dmask.isEmpty()) {
            parser.showHelp(-1);
        }

        if (nmask.size() != dmask.size()) {
            parser.showHelp(-1);
        }

        QRegExp rx("[01]+");
        if (!rx.exactMatch(nmask) || !rx.exactMatch(dmask)) {
            parser.showHelp(-1);
        }

        if (active.isEmpty()) {
            active = "OPAMP";
        }

        if (xover.isEmpty()) {
            xover = "ONEPOINT";
        }
        bool pcost = parser.isSet("p");
        bool gcost = parser.isSet("g");
        bool adaptive = parser.isSet("m");
        return synthesize(outfile, nmask, dmask, active,
                          xover, rc, cc, pcost, gcost, adaptive);
    } else {
        parser.showHelp(-1);
    }
    return app.exec();
}
