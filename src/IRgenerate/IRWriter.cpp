#include "IRWriter.h"

IRWriter::IRWriter()
    : funcDepth(0), tempCounter(1), labelCounter(0), strConstCounter(0) {}

void IRWriter::emitDecl(const string& text) {
    decls << text << "\n";
}

string IRWriter::emitStrConst(const string& val) {
    string name = "@str" + to_string(strConstCounter++);
    int len = val.length() + 1;
    globals << name << " = constant [" << len << " x i8] c\"" << val << "\\00\"\n";
    return name;
}

void IRWriter::emitLine(const string& text) {
    if (funcDepth > 0)
        funcDefs << "\t" << text << "\n";
    else
        body << "\t" << text << "\n";
}

void IRWriter::emitRaw(const string& text) {
    if (funcDepth > 0)
        funcDefs << text;
    else
        body << text;
}

string IRWriter::newTemp() {
    return "%t" + to_string(tempCounter++);
}

string IRWriter::newLabel(const string& prefix) {
    return prefix + to_string(labelCounter++);
}

void IRWriter::beginFunctionDef(const string& name, const string& returnType,
                                 const vector<pair<string, string>>& params) {
    funcDefs << "define " << returnType << " @" << name << "(";
    for (size_t i = 0; i < params.size(); i++) {
        funcDefs << params[i].first << " %" << params[i].second;
        if (i + 1 < params.size()) funcDefs << ", ";
    }
    funcDefs << ") {\n";
    funcDefs << "main:\n";
    funcDepth++;
}

void IRWriter::endFunctionDef() {
    funcDepth--;
    funcDefs << "}\n";
}

string IRWriter::build() const {
    return decls.str() + "\n" + globals.str() + "\n" + body.str() + "\n" + funcDefs.str();
}
