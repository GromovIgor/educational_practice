#ifndef DECOMPRESSION_H
#define DECOMPRESSION_H

#include <QDialog>

namespace Ui {
class Decompression;
}

class Decompression : public QDialog
{
    Q_OBJECT
    
public:
    explicit Decompression(QWidget *parent = 0);
    ~Decompression();

    QString getInputFileName();

    void huffmanDecode(const char * inputFile);
    
private:
    Ui::Decompression *ui;
    QString inputFileName;
    void outputFilePath(const char * path, char * outputPath, const char * fileExtension);
    void getOutputFileName();
    void showDoneMessage(const char * msg);

    struct HuffNode
    {
        unsigned char charCode;
        unsigned int freq;
        bool leaf;
        HuffNode * next;
        HuffNode * left;
        HuffNode * right;
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

    void buildNodeList(HuffNode ** nodeList, HuffFreq * hFreq, unsigned int numOfFreq);
    void addToNodeList(HuffNode ** nodeList, HuffNode * newNode);
    void buildHuffTree(HuffNode ** nodeList);
    void writeDecodedData(FILE * src, FILE * dest, HuffNode * rootTree, unsigned int fileSize);
    void freeHuffTree(HuffNode * treeRoot);

public slots:

    void browseInputFile();
    void decompressFile();
};

#endif // DECOMPRESSION_H
