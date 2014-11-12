#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <QDialog>

#define MAX_CODE_SIZE 32
#define CHARS_LIMIT 256

namespace Ui {
class Compression;
}

class Compression : public QDialog
{
    Q_OBJECT
    
public:
    explicit Compression(QWidget *parent = 0);
    ~Compression();
    QString getInputFileName();
    void huffmanEncode(const char* inputFile);
    unsigned int getFileSize(FILE * src);
    unsigned int calcNumOfFreq(unsigned int * freqList);

private:
    Ui::Compression *ui;
    QString inputFileName;
    void outputFilePath(const char * path, char * outputPath, const char * fileExtension);
    void showDoneMessage(const char * msg);

    struct HuffNode
    {
        unsigned int freq;
        unsigned char charCode;
        bool leaf;
        HuffNode * next;
        HuffNode * left;
        HuffNode * right;
    };

    struct HuffCode
    {
        unsigned char code[MAX_CODE_SIZE];
        unsigned int length;
    };

    struct HuffHeader
    {
        unsigned int numOfFreq;
        unsigned int fileSize;
    };

    struct HuffFreq
    {
        unsigned int freq;
        unsigned char charCode;
    };

    void calcCharFreq(FILE * src, unsigned int * freqList);
    void buildNodeList(HuffNode ** nodeList, unsigned int * freqList);
    void addToNodeList(HuffNode ** nodeList, HuffNode * newNode);
    void buildHuffTree(HuffNode ** nodeList);
    bool buildHuffCode(HuffNode * treeRoot, HuffCode * hCode, unsigned char goalChar);
    void writeHeader(FILE * dest, HuffHeader hHeader, unsigned int numOfFreq, unsigned int fileSize);
    void writeFreq(FILE * dest, unsigned int * freqList, HuffFreq hFreq);
    void writeEncodedData(FILE * src, FILE * dest, HuffCode * huffCodeTable, unsigned int fileSize);
    void freeHuffTree(HuffNode * treeRoot);

public slots:

    void browseInputFile();
    void compressFile();
};

#endif // COMPRESSION_H
