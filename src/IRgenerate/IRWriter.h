#ifndef IRWRITER_H
#define IRWRITER_H

#include <string>
#include <vector>
#include <sstream>
#include <utility>

using namespace std;

class IRWriter {
public:
    IRWriter();

    void emitDecl(const string& text);
    string emitStrConst(const string& val);
    void emitLine(const string& text);
    void emitRaw(const string& text);
    string newTemp();
    string newLabel(const string& prefix);
    void beginFunctionDef(const string& name, const string& returnType,
                          const vector<pair<string, string>>& params);
    void endFunctionDef();
    string build() const;

private:
    ostringstream decls;
    ostringstream globals;
    ostringstream body;
    ostringstream funcDefs;
    int funcDepth;
    int tempCounter;
    int labelCounter;
    int strConstCounter;
};

#endif
