#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>

//if comparing == 1 print error message if(compared1 == comared2)
//if comparing != 1 print error message if(compared1 != compared)
#define PRINT_TO_LOG_FILE_IF_NOT_EQUAL(compared1, compared2, whatToPrint) \
                            if ((compared1) != (compared2)) \
                                fprintf(stk->logFile, " %s \n", (whatToPrint));

#ifndef Size_t
#define Size_t int
#endif

#ifndef Elem_t
#define Elem_t int
#endif

#ifdef STACK_PROTECT
#define ON_STACK_PROTECT(code) code
#else
#define ON_STACK_PROTECT(code)
#endif

//typedef int bool;
typedef unsigned long long StackCanary;
typedef unsigned long long StackHash_t;

#define MAX_STACK_FILE_NAME 50

//int _mutexCapacityOfStacks = 0;
int _capacityOfStacks = 0;


enum errnumbers
{
    FILEERROR = 13
};

enum stackDefenders
{
    CANARY1 = 0xBABABABABABABABA,
    CANARY2 = 0xFFFFBBBBAAAAFFFF
};

typedef struct Stack
{
    ON_STACK_PROTECT(StackCanary canary1);
    ON_STACK_PROTECT(StackHash_t hash);
    Elem_t* boof;
    Size_t currElem;
    Size_t size;
    FILE* logFile;
    ON_STACK_PROTECT(StackCanary canary2);
} Stack;

int AssertStackConstruct(Stack* stk, FILE* logFile)
{
    if (stk == NULL) {
        fprintf(logFile, "\n Wrong input in construct stack \n");
        return 0;
    }
    return 1;
}

StackHash_t Rol(StackHash_t value)
{
    return (value << 1) | (value >> (8 * sizeof(value) - 1));
}

StackHash_t _HashFuncOfStack(const Stack* stk)
{
    StackHash_t hash = 0;
    unsigned char* stackInBytes = (unsigned char*) stk;

    for (int i = sizeof(StackCanary) + sizeof(StackHash_t); i < sizeof(Stack) - sizeof(StackCanary); i++) {
        hash = Rol(hash) ^ stackInBytes[i];
    }
    return hash;
}

int _StringCapacityOfStacks(char* stringNumber) {
    itoa(_capacityOfStacks, stringNumber, 10);
    return 1;
};

int StackConstructFull(Stack* stk, size_t capacity, char* stkLogFileName)
{
    FILE* logFile;
    logFile = fopen(stkLogFileName, "w");
    fprintf(logFile, "\n \n BEGIN OF STK ERRORS \n \n ");

    if (!AssertStackConstruct(stk, logFile) /*!= 1*/)
        return 0;

    ON_STACK_PROTECT(stk->canary1 = CANARY1;
                     stk->canary2 = CANARY2);
    stk->size = capacity;
    stk->currElem = -1;
    stk->boof = (Elem_t*)calloc(capacity, sizeof(Elem_t));
    stk->logFile = logFile;
    ON_STACK_PROTECT(stk->hash = _HashFuncOfStack(stk));

    if (!AssertStackConstruct(stk, logFile) /*!= 1*/)
        return 0;

    fprintf(stk->logFile, "\n Constructed well \n");
    _capacityOfStacks++;
    return 1;
}
int StackConstruct(Stack* stk, size_t capacity)
{
    char stackLogName[MAX_STACK_FILE_NAME];

    _StringCapacityOfStacks(stackLogName);
    sprintf(stackLogName, "StackLog%s.txt", stackLogName);

    return StackConstructFull(stk, capacity, stackLogName);
}

int StackErrors(Stack* stk)
{
    if (stk->logFile == NULL) {
        stk->logFile = fopen("StackLog.txt", "w");
        fprintf(stk->logFile, "\n \n BEGIN OF STK ERRORS  \n (No logFile, New Created in %s) \n ", __PRETTY_FUNCTION__ );
    }
    ON_STACK_PROTECT(
    PRINT_TO_LOG_FILE_IF_NOT_EQUAL(stk->canary1, CANARY1, \
                        "Canary1 has been changed. Stack can be broken");
    PRINT_TO_LOG_FILE_IF_NOT_EQUAL(stk->canary2, CANARY2, \
                        "Canary2 has been changed. Stack can be broken");
    PRINT_TO_LOG_FILE_IF_NOT_EQUAL(stk->hash, _HashFuncOfStack(stk), "HASH has been changed. Stack can be broken");
    )
    return 0;
}

void StackDelete(Stack* stk)
{
    fprintf(stk->logFile, "\n \n END OF STK ERRORS \n \n");

    fclose(stk->logFile);

    free(stk->boof);
    stk = NULL;
}

/*
void StackPop(Stack* stk)
{
    if (StackErrors(stk))
        return;


}

void StackResize(Stack* stk, int type) //type = -1 if stack decreasing, type = 1 if stack increasing
{

}
 */

int main()
{
    Stack MyStack;
    int i = StackConstructFull(&MyStack, 3, "stklog.txt");
    ON_STACK_PROTECT(
    if (_HashFuncOfStack(&MyStack) != MyStack.hash)
        printf("Bad Stack %llu, %llu", _HashFuncOfStack(&MyStack), MyStack.hash);
    )
    StackDelete(&MyStack);
    return 0;
}


