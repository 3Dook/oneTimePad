// Author: Duc Doan
// Date: 03032020

//Used like other block programs
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// for memset buffer
#include <string.h>


/***************************************
This function will take in an integer and create a random assortment of character(1-27)
Captial Letters and space. It will then send to the stdout.
***************************************/
void main(int argc, char *argv[]){
    int num = atoi(argv[1]);
    srand(time(NULL));
    // note to self that c does not accept int argument, but string, so you MUST convert it
    // https://stackoverflow.com/questions/43160958/passing-a-integer-through-command-line-in-c/43161028

    //printf("You passed the following: %s\n", argv[1]);
    //printf("You passed the following: %d\n", num);

    // We will create an array with all the proper values of ASCII characters, then... 
    // use random (0 - 27) to add each specific number from the array into new array.

    // two arrays, one to hold all digits and one to add to.
    int dict[26];
    int holder[num];

    int i = 0;
    int k = 65;
    //ASCCI cap are 65(A) - 90(Z) ; space = 32
    for(i=0; i < 26; i++){
        dict[i] = k;
        k++;
    }
    dict[26] = 32;
    // print out content
   /*
    for(i=0; i < 27; i++){
        printf("Array Content @%d: %d\n", i, dict[i]);
    }
    */
    for(i=0; i < num; i++){
        //ASCCI cap are 65(A) - 90(Z) ; space = 32
        k = rand() % 27;
        holder[i] = dict[k];
    }
    /*
    for(i=0; i < num; i++){
        printf("Holder Content @%d: %d\n", i, holder[i]);
    }
    */
    char buffer[num+1];
    memset(buffer, '\0', sizeof(buffer));
    // TODO: CONVERT EACH INT INTO STRING, 
    char c;
    for(i=0; i < num; i++){
        c = holder[i];
        buffer[i] = c;
    }
    // note that that It is not adding the \n character
    //buffer[num+1] = '\n';
    //printf("BUFFER IS NOW %s\nwith size of %i", buffer, sizeof(buffer)/sizeof(buffer[0]));
    // just gonna add the new line during the stdout
    printf("%s\n", buffer);
}