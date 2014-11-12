#include "compression.h"
#include "ui_compression.h"
#include "QFileDialog"
#include "QMessageBox"
#include "stdio.h"
#include "stdlib.h"
#include "QProgressDialog"
#include <QDebug>

Compression::Compression(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Compression)
{
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(browseInputFile()));
    connect(ui->compres_btn, SIGNAL(clicked()), this, SLOT(compressFile()));
}

Compression::~Compression()
{
    delete ui;
}

void Compression::browseInputFile()
{
    inputFileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Текстовый файл (*.txt)"));
    ui->inputFile->setText(inputFileName);
}

void Compression::outputFilePath(const char *path, char * outputPath, const char *fileExtension)
{
    int i;
    const int pathLength = strlen(path);

    for(i=0; i<pathLength-4; i++)
    {
        outputPath[i] = path[i];
    }

    strcat(outputPath, fileExtension);
}

void Compression::compressFile()
{
    if (inputFileName == "")
    {
        showDoneMessage("Сначала выберите файл.");
    }
    else
    {
        QByteArray byteArray1 = inputFileName.toUtf8();
        const char* inputFile = byteArray1.constData();

        huffmanEncode(inputFile);
    }
}

void Compression::showDoneMessage(const char * msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

unsigned int Compression::getFileSize(FILE * src)
{
    unsigned int fileSize = 0;
    unsigned int c;

    while((c = fgetc(src)) != EOF)
    {
        fileSize++;
    }
    rewind(src);

    return fileSize;
}

void Compression::calcCharFreq(FILE * src, unsigned int * freqList)
{
    unsigned int c;

    while((c = fgetc(src)) != EOF)
    {
        freqList[c]++;
    }
    rewind(src);
}

unsigned int Compression::calcNumOfFreq(unsigned int * freqList)
{
    unsigned int i;
    unsigned int numOfFreq = 0;

    for(i=0; i<CHARS_LIMIT; i++)
    {
        if(freqList[i] > 0)
        {
            numOfFreq++;
        }
    }

    return numOfFreq;
}

void Compression::buildNodeList(HuffNode ** nodeList, unsigned int * freqList)
{
    unsigned int i;
    HuffNode * newNode;

    for(i=0; i<CHARS_LIMIT; i++)
    {
        if(freqList[i] > 0)
        {
            newNode = (HuffNode *)calloc(1, sizeof(HuffNode));
            newNode->charCode = i;
            newNode->freq = freqList[i];
            newNode->next = NULL;
            newNode->left = NULL;
            newNode->right = NULL;
            newNode->leaf = true;

            addToNodeList(nodeList, newNode);
        }
    }
}

void Compression::addToNodeList(HuffNode ** nodeList, HuffNode * newNode)
{
    HuffNode * prevNode = NULL;
    HuffNode * currNode = *nodeList;

    while(currNode != NULL && currNode->freq < newNode->freq)
    {
        prevNode = currNode;
        currNode = prevNode->next;
    }

    newNode->next = currNode;

    if(prevNode == NULL)
    {
        *nodeList = newNode;
    }
    else
    {
        prevNode->next = newNode;
    }

  //  qDebug() << newNode->charCode << newNode->freq;
}

void Compression::buildHuffTree(HuffNode ** nodeList)
{
    HuffNode * leftNode, * rightNode;
    HuffNode * newNode;

    while((*nodeList)->next)
    {
        leftNode = *nodeList;
        *nodeList = leftNode->next;

        rightNode = *nodeList;
        *nodeList = rightNode->next;

        newNode = (HuffNode *)calloc(1, sizeof(HuffNode));
        newNode->charCode = 0;
        newNode->freq = leftNode->freq + rightNode->freq;
        newNode->next = NULL;
        newNode->left = leftNode;
        newNode->right = rightNode;
        newNode->leaf = false;

        addToNodeList(nodeList, newNode);

        qDebug() << newNode->charCode << newNode->freq;
    }
}

bool Compression::buildHuffCode(HuffNode * treeRoot, HuffCode * hCode, unsigned char goalChar)
{
    if(treeRoot->charCode == goalChar && treeRoot->leaf)
    {
        return true;
    }

    if(treeRoot->left)
    {
        hCode->code[hCode->length] = '0';
        hCode->length++;

        if(hCode->length == MAX_CODE_SIZE)
        {
            printf("Слиишком болшой размер.");
            return false;
        }

        if(buildHuffCode(treeRoot->left, hCode, goalChar))
        {
            hCode->code[hCode->length] = 0;
            return true;
        }
        else
        {
            hCode->length--;
            hCode->code[hCode->length] = 0;
        }
    }

    if(treeRoot->right)
    {
        hCode->code[hCode->length] = '1';
        hCode->length++;

        if(buildHuffCode(treeRoot->right, hCode, goalChar))
        {
            return true;
        }
        else
        {
            hCode->length--;
            hCode->code[hCode->length] = 0;
        }
    }

    return false;
}

void Compression::writeHeader(FILE * dest, HuffHeader hHeader, unsigned int numOfFreq, unsigned int fileSize)
{
    hHeader.numOfFreq = numOfFreq;
    hHeader.fileSize = fileSize;

    fwrite(&hHeader, sizeof(hHeader), 1, dest);
}

void Compression::writeFreq(FILE * dest, unsigned int * freqList, HuffFreq hFreq)
{
    unsigned int i;

    for(i=0; i<CHARS_LIMIT; i++)
    {
        if(freqList[i] > 0)
        {
            hFreq.charCode = i;
            hFreq.freq = freqList[i];

            fwrite(&hFreq, sizeof(HuffFreq), 1, dest);
        }
    }
}

void Compression::writeEncodedData(FILE * src, FILE * dest, HuffCode * huffCodeTable, unsigned int fileSize)
{
    unsigned int i, c;
    unsigned int bits = 0;
    char currChar = 0;
    HuffCode currCode;
    bool cancel = false;
    unsigned int interval = fileSize/100;
    int progress = 1;
    unsigned int bytes = 0;

    QProgressDialog progressDialog("Подождите...", "Отмена", 0, fileSize, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    while((c = fgetc(src)) != EOF)
    {
        bytes++;
        currCode = huffCodeTable[c];

        for(i=0; i<currCode.length; i++)
        {
            currChar = (currChar << 1) + (currCode.code[i] == '1' ? 1 : 0);
            bits++;

            if(bits == 8)
            {
                fputc(currChar, dest);
                currChar = 0;
                bits = 0;
            }
        }


        if(bytes > interval*progress)
        {
            progressDialog.setValue(progress);
            progress++;
        }

        if (progressDialog.wasCanceled())
        {
            cancel = true;
            showDoneMessage("Операция отменена.");
            break;
        }
    }

    if(bits > 0)
    {
        currChar = currChar << (8 - bits);
        fputc(currChar, dest);
    }

    progressDialog.setValue(fileSize);

    if(!cancel)
    {
        showDoneMessage("Успешное завершение.");
    }
}

void Compression::freeHuffTree(HuffNode * treeRoot)
{
    if(treeRoot)
    {
        freeHuffTree(treeRoot->left);
        freeHuffTree(treeRoot->right);

        free(treeRoot);
    }
}

void Compression::huffmanEncode(const char* inputFile)
{
    FILE * src = fopen(inputFile, "rb");

    char outputPath[1000];
    const char * fileExtension = ".bin";
    outputFilePath(inputFile, outputPath, fileExtension);
    FILE * dest = fopen(outputPath, "wb");

    if (src == NULL || dest == NULL)
    {
        printf("Не удается найти файл.");
        exit(EXIT_FAILURE);
    }

    unsigned int fileSize;
    fileSize = getFileSize(src);

    unsigned int * freqList;
    freqList = (unsigned int *)calloc(CHARS_LIMIT, sizeof(unsigned int));
    calcCharFreq(src, freqList);

    unsigned int numOfFreq;
    numOfFreq = calcNumOfFreq(freqList);

    HuffNode * nodeList = NULL;
    buildNodeList(&nodeList, freqList);

    buildHuffTree(&nodeList);
    HuffNode * treeRoot = nodeList;

    unsigned int i;
    HuffCode newCode;
    HuffCode * huffCodeTable;
    huffCodeTable = (HuffCode *)calloc(CHARS_LIMIT, sizeof(HuffCode));
    for(i=0; i<CHARS_LIMIT; i++)
    {
        if(freqList[i] > 0)
        {
            newCode.length = 0;
            buildHuffCode(treeRoot, &newCode, i);
            huffCodeTable[i] = newCode;
        }
    }

    HuffHeader hHeader;
    writeHeader(dest, hHeader, numOfFreq, fileSize);

    HuffFreq hFreq;
    writeFreq(dest, freqList, hFreq);

    writeEncodedData(src, dest, huffCodeTable, fileSize);

    freeHuffTree(treeRoot);
    treeRoot = NULL;
    free(huffCodeTable);
    free(freqList);

    fclose(src);
    fclose(dest);
}


