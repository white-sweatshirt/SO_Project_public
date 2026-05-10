#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <sys/sem.h>
#include <limits.h>
#include <errno.h>

// name function#x is reserved for child #x

#define FIRST_SEMAPHORE 0 // semop operates on
#define ONE_OPERATION 1
#define WAIT_OP 0
union semun
{
    // union for orders for semaphore, only one fild is present at time
    // definition copy pasted from man page
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
};
void readCharactersFromFileUnInterrupted(int desc, void *buffer, int sizeBuf, int *amountRead);
int createSemaphore(int semNumber, key_t semKey)
{
    // creates semaphore and sets initial value of it to zero
    int semaphoreId;
    if ((semaphoreId = semget(semKey, semNumber, IPC_CREAT | 0766)) == -1)
    {
        perror("semaphore creation");
        return -1;
    }
    else
        return semaphoreId;
}
int setInitialValueOfSemaphore(int semId, int value)
{

    union semun controlOperation;
    controlOperation.val = value;
    // semctl as last argument acepts union that has field specyfic to its needs
    if (semctl(semId, 0, SETVAL, controlOperation) == -1)
    {
        perror("blad ustawnia semclt");
        return -1;
    }
    else
        return 0;
}
int killSemaphore(int semId, int semNumber)
{
    union semun ignoredArgument;
    int op = 0;
    if ((op = semctl(semId, semNumber, IPC_RMID, ignoredArgument)) == -1)
    {
        perror("semaphore deletion");
        return -1;
    }
    else
        return op;
}
/*written and produced by Me(whiteSweatshirt i.e. Franciszek Wawer)*/
/*Definions for clarity*/

int waitForSemaphore(int semId)
{
    struct sembuf semOperation;
    semOperation.sem_flg = 0;
    semOperation.sem_op = WAIT_OP;
    semOperation.sem_num = FIRST_SEMAPHORE;
    int resultOfOperation;
    do
    {
        errno = 0;
        resultOfOperation = semop(semId, &semOperation, ONE_OPERATION);

    } while (errno == EINTR);
    if (errno != 0 && errno != EINTR)
        perror("wait for semaphore");
    return resultOfOperation;
}
#define RISE_BY_ONE 1
void raiseSemaphore(int semId)
{
    // raiseSemaphore i.e. P()
    struct sembuf rising;
    rising.sem_flg = 0;
    rising.sem_num = FIRST_SEMAPHORE;
    rising.sem_op = RISE_BY_ONE;
    int opResult;
    do
    {
        errno = 0;
        opResult = semop(semId, &rising, ONE_OPERATION);
    } while (errno == EINTR);
    if (errno != EINTR && errno != 0)
        perror("raise sem");
}
#define FALL_OP -1
void fallSemaphore(int semId)
{
    // fallSemaphore i.e. V()
    struct sembuf fall;
    fall.sem_flg = 0;
    fall.sem_op = FALL_OP;
    fall.sem_num = FIRST_SEMAPHORE;
    int opResult = 0;
    do
    {
        errno = 0;
        opResult = semop(semId, &fall, ONE_OPERATION);
    } while (errno == EINTR);

    if (errno != EINTR && errno != 0)
        perror("falling sem");
}
struct msgbuffer
{

    long mtype; // type of messeges
    int mtext;  // data
};
int openQue(int superKey)
{

    int queId;
    if ((queId = msgget(superKey, IPC_CREAT | 0666)) == -1)
        return -1;
    else
        return queId;
}
int sendMessage(int messeageQueId, struct msgbuffer *pmes)
{
    int opResult;
    do
    {
        errno = 0;
        opResult = msgsnd(messeageQueId, pmes, sizeof(pmes->mtext), 0);
    } while (errno == EINTR);

    if (errno != EINTR && errno != 0)
        perror("message sending error");

    return opResult;
}
int reciveMessege(int queId, struct msgbuffer *pmes)
{
    int resultOp;
    do
    {
        errno = 0;
        resultOp = msgrcv(queId, pmes, sizeof(pmes->mtext), 1, 0);
    } while (errno == EINTR);

    if (errno != EINTR && errno != 0)
        perror("failure to read message!");

    return resultOp;
}
void killMessageQue(int messageQueId)
{
    if (msgctl(messageQueId, IPC_RMID, NULL) == -1)
        perror("messege que deletion error!");
}
#define TEMP_FILE "./transitionFolder/helpFile.txt"
#define NUMBER_OF_PROCESSES 3
enum processNumbers
{
    FIRST,
    SECUND,
    THIRD
};
static pid_t childPIDs[NUMBER_OF_PROCESSES];
#define BUF_SIZE 50
void printCommunicatWithFlush(char *communicat)
{
    printf("%s", communicat);
    fflush(stdout);
}
void reverse(char *s)
{
    // Assumptions: this pointer is to string of characters
    // Ended with '\0'
    char *walker = s;
    while (*walker++ != '\0')
        ;
    char temp = 0;
    walker--;
    while (walker > s)
    {
        temp = *s;
        *s++ = *(--walker);
        *walker = temp;
    } // protection from going to far i.e changing charater before with one later
}
void addSpaceAtEnd(char buff[], int *end)
{
    // end is position of '\0'
    buff[*end] = ' ';
    buff[++*end] = '\0';
}

int prepereNumberInHexForDepature(int number, char buffor[])
{

    int temp, i = 0;
    do
    {
        errno = 0;

        buffor[i++] = '0';
        buffor[i++] = 'x';
        if (number == 0)
            buffor[i++] = '0';
        for (; i < BUF_SIZE && number > 0; i++)
        {
            temp = number % 16;
            if (temp < 10)
                buffor[i] = temp + '0';
            else
                buffor[i] = (temp - 10) + 'A';
            number = number / 16;
        }

        buffor[i] = '\0';
        reverse(buffor + 2);
        addSpaceAtEnd(buffor, &i);
    } while (errno == EINTR);
    return i;
}
void sendHexByFile(int desc, char buf[])
{
    int i = 0;
    for (i = 0; *(buf + i) != '\0'; i++)
        write(desc, buf + i, sizeof(char));
}
int readHexFromFile(int desc, char buf[], int sizBuf)
{
    int readenCharacters = 0;
    int i = 0;
    for (i = 0; read(desc, buf + i, sizeof(char)) > 0; i++)
        ;
    *(buf + i) = '\0';
    readenCharacters = i;
    return readenCharacters;
}
int sendBufforByQue(int queId, int buf[])
{
    struct msgbuffer mes;
    int i = 0;
    for (i = 0; *(buf + i) != '\0'; i++)
    {
        mes.mtext = buf[i];
        fflush(stdout);
        mes.mtype = 1;
        if (sendMessage(queId, &mes) == -1)
            return -1;
    }
    return 0;
}

void ignoreToNextEnter()
{
    int c = -1;
    while ((c = getchar()) != EOF && c != '\n')
        ;
}
void showOptionsFromWitchToTakeCharacters(void)
{
    printf("podaj zrodlo danych\n");
    printf("1. Iteractive Mode i.e wpisz z klawiatury\n");
    printf("2. File , wowczas podaj nazwe plkiu\n");
    printf("3. /dev/urandom losowe liczby generowane przez system\n");
    printf("w przypadku wybrania opcji 3 przerwiji program urzywacjac SIGINT\n");
    printf(">>");
}
int getStandardChoiceResult(void (*showMenu)(void), int lowerLimit, int upperLimit)
{
    char choice;
    int readenCharacters;
    do
    {
        showMenu();
        readCharactersFromFileUnInterrupted(STDIN_FILENO, &choice, sizeof(char), &readenCharacters);
        ignoreToNextEnter();
    } while (choice < lowerLimit || choice > upperLimit);
    return choice;
}
#define DEV_URANDOM_OPTION "/dev/urandom"
#define CHOICE_SIZE 100
int isConvertingToHex = 1;
void readCharactersFromFileUnInterrupted(int desc, void *buffer, int sizeBuf, int *amountRead)
{
    do
    {
        // necessery loop without it interruption causes read to fail
        // and read gives us negative descriptor therby terminiatig loop
        // and by extension program
        errno = 0;
        *amountRead = read(desc, buffer, sizeBuf);
    } while (errno == EINTR);
}
int writeCharactersToFileUnInterrupted(int desc, void *buffer, int sizeBuf)
{
    int charactersWrriten = 0;
    do
    {
        // necessery loop without it interruption causes read to fail
        // and read gives us negative descriptor therby terminiatig loop
        // and by extension program
        errno = 0;
        charactersWrriten = write(desc, buffer, sizeBuf);
    } while (errno == EINTR);
    return charactersWrriten;
}
int openFileToWriteUnInterrupted(char name[])
{
    int desc = -1;
    do
    {
        errno = 0;
        desc = open(name, O_WRONLY | O_TRUNC);
    } while (errno == EINTR);
    if (errno != 0 && errno != EINTR)
        perror("wypisywanie znaczkow");
    return desc;
}
int openFileToReading(char name[])
{
    int desc = -1;
    do
    {
        errno = 0;
        desc = open(name, O_RDONLY);
    } while (errno == EINTR);
    return desc;
}
void closeFile(int desc)

{
    do
    {
        errno = 0;
        close(desc);
    } while (errno == EINTR);
}
void putEndOfString(int table[], int i)
{
    table[i] = '\0';
}
void readChractersIntoIntigerTable(int desc, int buf[], int maxCharacters, int *charactersReadInFunction)
{
    // protocol for communication is such that file starts with size of int and value that means
    // amount of characters in file
    int charactersToRead = 1, temp;
    int filler;
    *charactersReadInFunction = 0;

    readCharactersFromFileUnInterrupted(desc, &charactersToRead, sizeof(int), &filler);
    if (filler < sizeof(int) || filler > sizeof(int))
        printCommunicatWithFlush("fuck this shit");
    for (int i = 0; i < charactersToRead && i < maxCharacters; i++)
        do
        {
            errno = 0;
            buf[i] = 0;
            readCharactersFromFileUnInterrupted(desc, buf + i, sizeof(char), &temp);
            *charactersReadInFunction += temp;
        } while (errno == EINTR);
}
void sendEnterBeforeText(int desc, int sizeOfOrginal, int *i)
{
    // protocol for communication is such that file starts with size of int and value that means
    // amount of characters in file
    int placeForOne = 0;
    if (*i >= 14)
    {
        placeForOne = sizeOfOrginal + 1;
        writeCharactersToFileUnInterrupted(desc, &placeForOne, sizeof(int));
        placeForOne = '\n';
        writeCharactersToFileUnInterrupted(desc, &placeForOne, sizeof(char));
        *i = 0;
    }
    else
    {
        writeCharactersToFileUnInterrupted(desc, &sizeOfOrginal, sizeof(int));
        (*i)++;
    }
}
void getFileName(char name[], int max)
{
    int i = 0, dumy;
    for (i = 0; i < max; i++)
    {
        readCharactersFromFileUnInterrupted(STDIN_FILENO, name + i, sizeof(char), &dumy);
        if (name[i] == '\n')
            break;
    }
    name[i] = '\0';
}
void function1(int semReading, int semWriting)
{
    int desc = 0, descForTransmiting = 0;
    int c, placeForOne = 0;
    char choice = 1;
    int amountRead = 0, i = 0;
    char buffer[BUF_SIZE];
    char fileOfChoice[CHOICE_SIZE];
    int conversionMod;
    int lenghtOfBuffer;
    choice = getStandardChoiceResult(showOptionsFromWitchToTakeCharacters, '1', '3');
    if (choice == '1')
        desc = STDIN_FILENO;
    else if (choice == '2')
    {
        printCommunicatWithFlush("podaj plik: ");
        getFileName(fileOfChoice, CHOICE_SIZE - 1);
        fileOfChoice[CHOICE_SIZE - 1] = '\0';
        desc = open(fileOfChoice, O_RDONLY);
    }
    else
        desc = open(DEV_URANDOM_OPTION, O_RDONLY);
    do
    {
        c = 0;
        buffer[0] = '\0';
        waitForSemaphore(semWriting);
        conversionMod = isConvertingToHex;
        readCharactersFromFileUnInterrupted(desc, &c, sizeof(char), &amountRead);
        descForTransmiting = openFileToWriteUnInterrupted(TEMP_FILE);

        if (amountRead > 0)
        {
            if (conversionMod == 1)
            {
                lenghtOfBuffer = prepereNumberInHexForDepature(c, buffer);
                sendEnterBeforeText(descForTransmiting, lenghtOfBuffer, &i);
                writeCharactersToFileUnInterrupted(descForTransmiting, buffer, sizeof(char) * lenghtOfBuffer);
            }
            else
            {
                placeForOne = 1;
                writeCharactersToFileUnInterrupted(descForTransmiting, &placeForOne, sizeof(int));
                writeCharactersToFileUnInterrupted(descForTransmiting, &c, sizeof(char));
            }
            closeFile(descForTransmiting);
        }
        else
        {
            closeFile(descForTransmiting);
            unlink(TEMP_FILE);
        }
        raiseSemaphore(semWriting);
        fallSemaphore(semReading);

    } while (amountRead > 0);

    closeFile(desc);
}
void function2(int queId, int semReading, int semWriting)
{
    struct msgbuffer mes;
    int desc = 0;
    int c;
    int amountRead = 0;
    int buffer[BUF_SIZE];
    mes.mtype = 1;
    do
    {
        buffer[0] = '\0';
        waitForSemaphore(semReading);
        desc = openFileToReading(TEMP_FILE);
        if (desc <= 0)
            break;
        readChractersIntoIntigerTable(desc, buffer, BUF_SIZE, &amountRead);
        putEndOfString(buffer, amountRead);
        if (sendBufforByQue(queId, buffer) == -1)
            break;
        closeFile(desc);
        raiseSemaphore(semReading);
        fallSemaphore(semWriting);

    } while (amountRead > 0);
    fflush(stdout);
    mes.mtext = EOF;
    mes.mtype = 1;
    sendMessage(queId, &mes);
    sleep(3);
}
void function3(int queId)
{
    struct msgbuffer mesage;
    sleep(1);
    while (1)
    {
        reciveMessege(queId, &mesage);
        if (mesage.mtext == EOF)
        {
            puts("mamy koniec!");
            break;
        }
        fputc(mesage.mtext, stdout);
        fflush(stdout);
    }
    fputc('\n', stdout);
    killMessageQue(queId);
}
int semWriting, semReading, queId;
void mainHandlerForSIGINT(int signal)
{
    // ending work of program
    kill(childPIDs[FIRST], SIGQUIT);
    kill(childPIDs[SECUND], SIGQUIT);
    kill(childPIDs[THIRD], SIGQUIT);
    killSemaphore(semWriting, 0);
    killSemaphore(semReading, 0);
    killMessageQue(queId);
    system("rm -r -f transitionFolder");
    putc('\n', stdout);
    exit(0);
}

void mainHandlerForSIGUSR1(int signal)
{
    // used to stop program

    kill(childPIDs[FIRST], SIGSTOP);
    kill(childPIDs[SECUND], SIGSTOP);
    kill(childPIDs[THIRD], SIGSTOP);
    kill(getpid(), SIGSTOP);
}
void mainHandlerForSIGUSR2(int signal)
{
    // restoring program functionality

    sleep(1);
    kill(childPIDs[FIRST], SIGCONT);
    kill(childPIDs[SECUND], SIGCONT);
    kill(childPIDs[THIRD], SIGCONT);
}
void mainHandlerForSIGURG(int signal)
{
    // turnig on/off conversion to hex

    kill(childPIDs[FIRST], SIGXFSZ);
    kill(childPIDs[SECUND], SIGXFSZ);
    kill(childPIDs[THIRD], SIGXFSZ);
}
void childHandlerForSIGUSR1(int signal)
{
    // stop program
    kill(getppid(), SIGUSR1);
}
void childHandlerForSIGUSR2(int signal)
{
    // resumeProgram

    kill(getppid(), SIGCONT);
    kill(getppid(), SIGUSR2);
    kill(getpid(), SIGSTOP);
}
void childHandlerForSIGURG(int signal)
{
    // turning pm/off conversion to hex
    kill(getppid(), SIGURG);
}
void childHandlerForSIGXFSZ(int signal)
{
    isConvertingToHex = (isConvertingToHex == 0) ? 1 : 0;
}
void childHandlerForSIGINT(int signal)
{
    puts("otrzymalem sigInt");
    kill(getppid(), signal);
}
void setChildsSignalHandlers(struct sigaction *sigInt, struct sigaction *sigUsr1,
                             struct sigaction *sigUsr2, struct sigaction *sigUrg,
                             struct sigaction *sigXfsz)
{
    sigInt->sa_handler = childHandlerForSIGINT;
    sigUsr1->sa_handler = childHandlerForSIGUSR1;
    sigUsr2->sa_handler = childHandlerForSIGUSR2;
    sigUrg->sa_handler = childHandlerForSIGURG;
    sigXfsz->sa_handler = childHandlerForSIGXFSZ;
    sigInt->sa_flags = 0;
    sigUsr1->sa_flags = 0;
    sigUsr2->sa_flags = 0;
    sigUrg->sa_flags = 0;
    sigXfsz->sa_flags = 0;
    sigaction(SIGINT, sigInt, NULL);
    sigaction(SIGUSR1, sigUsr1, NULL);
    sigaction(SIGUSR2, sigUsr2, NULL);
    sigaction(SIGURG, sigUrg, NULL);
    sigaction(SIGXFSZ, sigXfsz, NULL);
}
#define FILE_FOR_SECUND_PROGRAM "./transitionFolder/PidFile.txt"
int main(void)
{
    struct sigaction sigInt, sigUsr1, sigUsr2, sigUrg, sigXfsz;
    setlocale(LC_ALL, "en_US.UTF8");
    umask(0);
    system("rm -r -f transitionFolder");
    mkdir("transitionFolder", 0776);
    creat(TEMP_FILE, 0776);
    creat(FILE_FOR_SECUND_PROGRAM, 0770);
    key_t superKey;

    superKey = ftok(".", 'm'); // m to poprostu liczba do hashu

    key_t semaphoreKey1, semKey2;

    // semWriting is about writing to helpFile.txt
    // the same is true for semReading+
    semaphoreKey1 = ftok(".", 'a');
    semKey2 = ftok(".", 'b');
    if ((semWriting = createSemaphore(1, semaphoreKey1)) == -1)
        return 1;
    if ((semReading = createSemaphore(1, semKey2)) == -1)
        return 1;
    if (setInitialValueOfSemaphore(semWriting, 0) == -1)
        return 2;
    if (setInitialValueOfSemaphore(semReading, 1) == -1)
        return 2;

    if ((queId = openQue(superKey)) == -1)
    {
        perror("fucked starting que");
        return 1;
    }
    if ((childPIDs[FIRST] = fork()) == 0)
    {
        setChildsSignalHandlers(&sigInt, &sigUsr1, &sigUsr2, &sigUrg, &sigXfsz);
        function1(semReading, semWriting);
        while (1)
            ;
        exit(0);
    }
    if ((childPIDs[SECUND] = fork()) == 0)
    {
        setChildsSignalHandlers(&sigInt, &sigUsr1, &sigUsr2, &sigUrg, &sigXfsz);
        function2(queId, semReading, semWriting);
        while (1)
            ;
        exit(0);
    }
    if ((childPIDs[THIRD] = fork()) == 0)
    {
        setChildsSignalHandlers(&sigInt, &sigUsr1, &sigUsr2, &sigUrg, &sigXfsz);
        function3(queId);
        fflush(stdout);
        childHandlerForSIGINT(SIGINT);
        while (1)
            ;
    }
    sigInt.sa_handler = mainHandlerForSIGINT;
    sigUsr1.sa_handler = mainHandlerForSIGUSR1;
    sigUsr2.sa_handler = mainHandlerForSIGUSR2;
    sigUrg.sa_handler = mainHandlerForSIGURG;
    sigaction(SIGINT, &sigInt, NULL);
    sigaction(SIGUSR1, &sigUsr1, NULL);
    sigaction(SIGUSR2, &sigUsr2, NULL);
    sigaction(SIGURG, &sigUrg, NULL);
    FILE *f = fopen(FILE_FOR_SECUND_PROGRAM, "w");
    fprintf(f, "%d %d %d %d", getpid(), childPIDs[FIRST], childPIDs[SECUND], childPIDs[THIRD]);
    fclose(f);
    while (1)
        ;
    return 0;
}