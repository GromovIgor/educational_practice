#include "decompression.h"
#include "ui_decompression.h"
#include "QFileDialog"
#include "QMessageBox"
#include "stdio.h"
#include "stdlib.h"
#include "QProgressDialog"

Decompression::Decompression(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Decompression)
{
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(browseInputFile()));
    connect(ui->decompress_btn, SIGNAL(clicked()), this, SLOT(decompressFile()));
}

Decompression::~Decompression()
{
    delete ui;
}

void Decompression::browseInputFile()
{
    inputFileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",
                                                 tr("Закодированный файл (*.bin)"));

    ui->inputFile->setText(inputFileName);
}

QString Decompression::getInputFileName()
{
    return inputFileName;
}

void Decompression::outputFilePath(const char *path, char * outputPath, const char *fileExtension)
{
    int i;
    const int pathLength = strlen(path);

    for(i=0; i<pathLength-4; i++)
    {
        outputPath[i] = path[i];
    }
    strcat(outputPath, fileExtension);
}

void Decompression::decompressFile()
{
    if (inputFileName == "")
    {
        showDoneMessage("Выберите файл.");
    }
    else
    {
       QByteArray byteArray1 = inputFileName.toUtf8();
        const char* inputFile = byteArray1.constData();
        huffmanDecode(inputFile);
    }
}

void Decompression::showDoneMessage(const char * msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

void Decompression::buildNodeList(HuffNode ** nodeList, HuffFreq * hFreq, unsigned int numOfFreq)
{
    unsigned int i;
    HuffNode * newNode;

    for (i = 0; i < numOfFreq; i++)
    {
        newNode = (HuffNode *) malloc(sizeof(HuffNode));
        newNode->charCode = hFreq[i].charCode;
        newNode->freq = hFreq[i].freq;
        newNode->next = NULL;
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->leaf = true;

        addToNodeList(nodeList, newNode);
    }
}

void Decompression::addToNodeList(HuffNode ** nodeList, HuffNode * newNode)
{
    HuffNode * prevNode = NULL;
    HuffNode * currNode = *nodeList;

    while ((currNode != NULL && currNode->freq < newNode->freq))
    {
        prevNode = currNode;
        currNode = prevNode->next;
    }

    newNode->next = currNode;

    if (prevNode == NULL)
    {
        *nodeList = newNode;
    }
    else
    {
        prevNode->next = newNode;
    }
}

void Decompression::buildHuffTree(HuffNode ** nodeList)
{
    HuffNode * newNode, * leftNode, * rightNode;

    while((*nodeList)->next != NULL)
    {
        newNode = (HuffNode *)malloc(sizeof(HuffNode));

        leftNode = *nodeList;
        *nodeList = leftNode->next;

        rightNode = *nodeList;
        *nodeList = rightNode->next;

        newNode->charCode = 0;
        newNode->freq = leftNode->freq + rightNode->freq;
        newNode->next = NULL;
        newNode->left = leftNode;
        newNode->right = rightNode;
        newNode->leaf = false;

        addToNodeList(nodeList, newNode);
    }
}

void Decompression::writeDecodedData(FILE * src, FILE * dest, HuffNode * rootTree, unsigned int fileSize)
{
    int bit = -1;
    unsigned int c;
    unsigned int bytesWritten = 0;
    HuffNode * currNode = rootTree;
    bool cancel = false;
    unsigned int interval = fileSize/100;
    int progress = 1;

    QProgressDialog progressDialog("Подождите...", "Отмена", 0, fileSize, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    while(bytesWritten < fileSize)
    {
        if(bit < 0)
        {
            c = fgetc(src);

            if(c == EOF)
            {
                break;
            }

            bit = 7;
        }

        if((c >> bit) & 1)
        {
            currNode = currNode->right;
        }
        else
        {
            currNode = currNode->left;
        }

        if(currNode->leaf)
        {
            fputc(currNode->charCode, dest);
            bytesWritten++;
            currNode = rootTree;
        }

        bit--;

        if(bytesWritten > interval*progress)
        {
            progressDialog.setValue(interval*progress);
            progress++;
        }

        if (progressDialog.wasCanceled())
        {
            cancel = true;
            showDoneMessage("Пренудительное завершение.");
            break;
        }
    }

    progressDialog.setValue(fileSize);

    if(!cancel)
    {
        showDoneMessage("Успешное завершение.");
    }
}

void Decompression::freeHuffTree(HuffNode * treeRoot)
{
    if(treeRoot)
    {
        freeHuffTree(treeRoot->left);
        freeHuffTree(treeRoot->right);

        free(treeRoot);
    }
}

void Decompression::huffmanDecode(const char * inputFile)
{
    FILE *src = fopen(inputFile, "rb");

    char outputPath[1000];
    const char * fileExtension = ".txt";
    outputFilePath(inputFile, outputPath, fileExtension);
    FILE * dest = fopen(outputPath, "wb");

    if (src == NULL)
    {
        fprintf(stderr, "Файл не найден.");
        exit(EXIT_FAILURE);
    }

    HuffHeader hHeader;
    fread(&hHeader, sizeof(hHeader), 1, src);

    HuffFreq *hFreq = (HuffFreq *)calloc(hHeader.numOfFreq, sizeof(HuffFreq));
    fread(hFreq, sizeof(HuffFreq), hHeader.numOfFreq, src);

    HuffNode * nodeList = NULL;
    buildNodeList(&nodeList, hFreq, hHeader.numOfFreq);

    buildHuffTree(&nodeList);

    writeDecodedData(src, dest, nodeList, hHeader.fileSize);

    freeHuffTree(nodeList);
    nodeList = NULL;
    free(hFreq);

    fclose(src);
    fclose(dest);
}
