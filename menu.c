#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// definitons copied from main process.
#define FILE_FOR_SECUND_PROGRAM "./transitionFolder/PidFile.txt"
#define NUMBER_OF_PROCESSES 3
enum processNumbers
{
    MAIN,
    FIRST,
    SECUND,
    THIRD
};
void ignoreToNextEnter()
{
    int c = -1;
    while ((c = getchar()) != EOF && c != '\n')
        ;
}
void showSignalChoiceMenu()
{
    printf("Menu operatora\n");
    printf("1. Wyslji sygnal terminujacy\n");
    printf("2. Wyslji sygnal wstrzymujacy prace\n");
    printf("3. Wyslji sygnal wznawiajacy prace\n");
    printf("4. Wyslji sygnal wstrzymujacy/wznawiajacy konwersje na hex\n");
    printf(">>");
    fflush(stdout);
}
void showProcessChoiceMenu()
{
    printf("Wybierz jeden z 4 mozliwych porcesow glownego programu\n");
    printf("1. proces macierzysty\n");
    printf("2. proces 1 wczytujacy\n");
    printf("3. proces 2 przekazujacy\n");
    printf("4. proces 3 pokazujacy na ekranie\n");
    printf(">>");
    fflush(stdout);
}
int getStandardChoiceResult(void (*showMenu)(void), int lowerLimit, int upperLimit)
{
    char choice;
    do
    {
        puts("************************");
        showMenu();
        choice = getchar();
        puts("************************");
        ignoreToNextEnter();
    } while (choice < lowerLimit || choice > upperLimit);
    return choice;
}
int giveSignalChosen(char choice)
{
    switch (choice)
    {
    case '1':
        return SIGINT;
        break;
    case '2':
        return SIGUSR1;
        break;
    case '3':
        return SIGUSR2;
        break;
    case '4':
        return SIGURG;
        break;
    default:
        break;
    }
}

int main()
{
    int mainProces[4];
    FILE *f = fopen(FILE_FOR_SECUND_PROGRAM, "r");
    if (!f)
    {
        perror("nie udane otwarcie za wczesne uruchominie");
        return 1;
    }
    fscanf(f, "%d %d %d %d", mainProces, mainProces + 1, mainProces + 2, mainProces + 3);
    fclose(f);
    unlink(FILE_FOR_SECUND_PROGRAM);
    int signalChoice, procesChoice;
    while (1)
    {
        signalChoice = getStandardChoiceResult(showSignalChoiceMenu, '1', '4');
        procesChoice = getStandardChoiceResult(showProcessChoiceMenu, '1', '4');
        puts("=======================");
        printf("chosen PID: %d", mainProces[procesChoice - '0']);
        printf("chosen signal: %d\n", giveSignalChosen(signalChoice));
        fflush(stdout);
        if (giveSignalChosen(signalChoice) == SIGUSR2)
            kill(mainProces[procesChoice - '0' - 1], SIGCONT);
        kill(mainProces[procesChoice - '0' - 1], giveSignalChosen(signalChoice));
        if (giveSignalChosen(signalChoice) == SIGINT)
            break;
    }
}