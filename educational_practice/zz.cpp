
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
