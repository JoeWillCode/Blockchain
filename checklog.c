#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>

static const char encodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int modTable[] = {0, 2, 1};

char* base64_encode(const unsigned char *data, size_t inputLength, size_t *outputLength) {
    *outputLength = 4 * ((inputLength + 2) / 3);
    char *encodedData = malloc(*outputLength);
    if (encodedData == NULL) return NULL;

    for (int i = 0, j = 0; i < inputLength;) {
        uint32_t octet_a = i < inputLength ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < inputLength ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < inputLength ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encodedData[j++] = encodingTable[(triple >> 3 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 2 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 1 * 6) & 0x3f];
        encodedData[j++] = encodingTable[(triple >> 0 * 6) & 0x3f];
    }

    for (int i = 0; i < modTable[inputLength % 3]; i++)
        encodedData[*outputLength - 1 - i] = '=';
    
    encodedData[*outputLength] = 0;
    return encodedData;
}

int main(int argc, char** argv)
{
    //error handling
    if(argc!=1)
    {
        printf("This program takes no input\n");
        exit(0);
    }
    if(access("log.txt", F_OK)!=0)
    {
        printf("failed: File \"log.txt\" is missing\n");
        exit(1);
    }
    if(access("loghead.txt", F_OK)!=0)
    {
        printf("failed: File \"loghead.txt\" is missing\n");
        exit(1);
    }

    //prepare files and buffer
    FILE* log;
    log = fopen("log.txt", "r");
    char buffer[44];

    //THIS IS TO CHECK FIRST LINE
    int characterCount = 32;
    fseek(log, 27, SEEK_CUR);
    fread(buffer, 5, 1, log);
    buffer[5] = '\0';
    if(strcmp(buffer, "begin")!=0)
    {
        printf("failed: corrupted or missing starting line in log\n");
        exit(1);
    }
    fseek(log, 0, SEEK_SET);

    int currentLine = 1;
    while(!feof(log))
    {
        characterCount = 0;
        int c = fgetc(log);
        characterCount++;
        while(c!='\n')
        {
            c = fgetc(log);
            characterCount++;
        }
        fseek(log, 0-characterCount, SEEK_CUR);
        char* input_text = malloc(characterCount+1);
        fread(input_text, characterCount-1, 1, log);
        input_text[characterCount-1] = '\0';
        fgetc(log);
        fseek(log, 27, SEEK_CUR);
        int success = fread(buffer, sizeof(buffer), 1, log);
        if(success>0)
        {
            fseek(log, -71, SEEK_CUR);
        }
        else
        {
            FILE* loghead;
            loghead = fopen("loghead.txt", "r");
            fread(buffer, sizeof(buffer), 1, loghead);
            fclose(loghead);
        }
        buffer[44] = '\0';

        //FOR TESTING
        //printf("hash:%s\ncharCount:%d\nLINEREAD:\n%s*", buffer, characterCount, input_text);
        //printf("\n%ld\n\n", strlen(input_text));

        // Initialize OpenSSL algorithms
        OpenSSL_add_all_algorithms();

        unsigned char hash[EVP_MAX_MD_SIZE]; // buffer for the hash
        unsigned int hash_len;

        // Create and initialize the context
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        EVP_DigestUpdate(ctx, input_text, strlen(input_text));
        EVP_DigestFinal_ex(ctx, hash, &hash_len);

        // Base64 encode the SHA256 hash
        size_t outputLength;
        char *base64_string = base64_encode(hash, hash_len, &outputLength);

        //check if the hashes match up
        if(strcmp(base64_string, buffer)!=0)
        {
            //FOR TESTING
            //printf("buffer V\n%s", buffer);
            //printf("\n%s\nbase64_string ^\n", base64_string);
            printf("failed: corruption on line %d\n", currentLine);
            exit(1);
        }

        // Clean up
        EVP_MD_CTX_free(ctx);
        free(base64_string);
        EVP_cleanup(); // Clean up the OpenSSL algorithms library
        free(input_text);
        currentLine++;
    }
    fclose(log);
    printf("vaild\n");
    exit(0);
}   