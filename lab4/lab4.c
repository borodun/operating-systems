#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node {
    char *string;
    struct node *next;
};

void freeNode(struct node *currentNode) {
    struct node *i = currentNode, *j;
    while (i != NULL) {
        j = i->next;
        free(i->string);
        free(i);
        i = j;
    }
}

struct node *addString(char *newLine) {
    struct node *newNode = malloc(sizeof(struct node));
    if (newNode == NULL) {
        return NULL;
    }
    newNode->string = malloc(strlen(newLine) + 1);
    if (newNode->string == NULL){
        free(newNode);
        return NULL;
    }
    strcpy(newNode->string, newLine);
    newNode->next = NULL;
    return (newNode);
}

int main() {
    char line[BUFSIZ];
    struct node *currentNode = malloc(sizeof(struct node));
    if(currentNode == NULL){
        printf("Error occurred with malloc\n");
        return -1;
    }
    struct node *headPtr = currentNode;

    printf("Type text. '.' to finish.\n");
    while (fgets(line, BUFSIZ, stdin)) {
        if (line[0] == '.') {
            break;
        }
        currentNode->next = addString(line);
        if (currentNode->next == NULL) {
            break;
        }
        currentNode = currentNode->next;
    }

    for (struct node *i = headPtr->next; i != NULL; i = i->next) {
        printf("%s", i->string);
    }

    freeNode(headPtr);
    return 0;
}