#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// cite: 6_5_prod_cons_pipeline.c x2
#define SIZE 1000
#define NUM_ITEMS 50

// cite: 6_5_prod_cons_pipeline.c
// Thread 1, called the Input Thread
  // reads in lines of characters from the standard input.
pthread_t input_t;
// Thread 2, called the Line Separator Thread
  // replaces every line separator in the input by a space.
pthread_t line_separator_t;
// Thread 3, called the Plus Sign thread
  // replaces every pair of plus signs, i.e., "++", by a "^".
pthread_t plus_sign_t;
// Thread 4, called the Output Thread
  // write this processed data to standard output as lines of exactly 80 characters.
pthread_t output_t;

// {stdin} -> (input_t) -> [buffer 1] -> (line_separator_t) -> [buffer 2] -> (plus_sign_t) -> [buffer 3] -> (output_t) -> {stdout}

// cite: 6_5_prod_cons_pipeline.c x3
// COMMENTS FOR DOCUMENTATION SINCE PREPROCESSOR MACROS ARE **COMMENT RESISTANT**
// TYPE buffer[SIZE];
// int  count = 0;                                     // Number of items in the buffer
// int  prod_idx = 0;                                  // Index where the input thread will put the next item
// int  con_idx = 0;                                   // Index where the square-root thread will pick up the next item
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Initialize the mutex for buffer 1
// pthread_cond_t full = PTHREAD_COND_INITIALIZER;     // Initialize the condition variable for buffer 1

// void put_buff(char item){
  //   // Lock the mutex before putting the item in the buffer
  //   pthread_mutex_lock(&mutex_1);
//   // Put the item in the buffer
//   buffer[prod_idx] = item;
//   // Increment the index where the next item will be put.
//   prod_idx = prod_idx + 1;
//   count++;
//   // Signal to the consumer that the buffer is no longer empty
//   pthread_cond_signal(&full);
  //   // Unlock the mutex
  //   pthread_mutex_unlock(&mutex);
// }

// char get_buff(){
//   // Lock the mutex before checking if the buffer has data
//   pthread_mutex_lock(&mutex);
//   while (count == 0)
//     // Buffer is empty. Wait for the producer to signal that the buffer has data
//     pthread_cond_wait(&full, &mutex);
//   char item = buffer[con_idx];
//   // Increment the index from which the item will be picked up
//   con_idx = con_idx + 1;
//   count--;
//   // Unlock the mutex
//   pthread_mutex_unlock(&mutex);
//   // Return the item
//   return item;
// }

// Template macro generates structure + helpers
#define BUFFER(name, type) \
type buffer_##name[50]; \
int  count_##name = 0; \
int  prod_idx_##name = 0; \
int  con_idx_##name = 0; \
pthread_mutex_t mutex_##name = PTHREAD_MUTEX_INITIALIZER; \
pthread_cond_t full_##name = PTHREAD_COND_INITIALIZER; \
\
void put_buff_##name(type item){ \
  pthread_mutex_lock(&mutex_##name); \
  buffer_##name[prod_idx_##name] = item; \
  prod_idx_##name = prod_idx_##name + 1; \
  count_##name++; \
  pthread_cond_signal(&full_##name); \
  pthread_mutex_unlock(&mutex_##name); \
} \
 \
type get_buff_##name(){ \
  pthread_mutex_lock(&mutex_##name); \
  while (count_##name == 0) \
    pthread_cond_wait(&full_##name, &mutex_##name); \
  type item = buffer_##name[con_idx_##name]; \
  con_idx_##name = con_idx_##name + 1; \
  count_##name--; \
  pthread_mutex_unlock(&mutex_##name); \
  return item; \
} \

// Buffer 1, shared resource between Input Thread and Line Separator thread
BUFFER(1, char*)
// Buffer 2, shared resource between Line Separator and Plus Sign thread
BUFFER(2, char*)
// Buffer 3, shared resource between Plus Sign and Output Thread thread
BUFFER(3, char*)

void *get_input(void *args) {
  char* item = malloc(1000*sizeof(char));
  while(strncmp(item, "STOP\n", 4)) {
    printf("Input: ");
    size_t n = 1000*sizeof(char);
    getline(&item, &n, stdin);
    fflush(stdin);
    put_buff_1(item);
  }
  return NULL;
}

void *get_line_separator_t(void *args) {
  char* item = malloc(1000*sizeof(char));
  while(strncmp(item, "STOP\n", 4)) {
    item = get_buff_1();

    for(int i = 0; i < strlen(item); i++) {
      if(item[i] == '\n') {
        item[i] = '-';
      }
    }

    put_buff_2(item);
  }
  return NULL;
}

void *get_plus_sign_t(void *args) {
  char* item = malloc(1000*sizeof(char));
  while(strncmp(item, "STOP\n", 4)) {
    item = get_buff_2();

    for(int i = 0; i < strlen(item); i++) {
      if((item[i] == '+') && ((i + 1) != strlen(item)) && (item[i+1] == '+')) {
        item[i] = '^';
        item[i+1] = '\0';
        char* temp = malloc(1000-i);
        strcpy(temp, &item[i+2]);
        strcat(item, temp);
        free(temp);
      }
    }

    put_buff_3(item);
  }
  return NULL;
}

char lineBuffer[1000] = "";
char outputBuffer[80] = "";

void *get_output_t(void *args) {
  char* item = malloc(1000*sizeof(char));
  while(strncmp(item, "STOP\n", 4)) {
    item = get_buff_3();

    // moving 0 index;
    int i = 0;

    while(strlen(&item[i])) {
      int j = 80-strlen(outputBuffer);
      strncat(outputBuffer, &item[i], (80-strlen(outputBuffer)));
      i += j;
      if(strlen(outputBuffer) >= 80) {
        printf("%s\n", outputBuffer);
        strcpy(outputBuffer, "");
      }
    }
  }
  return NULL;
}

int main() {
  for(int i = 0; i < 50; i++) {
    buffer_1[i] = malloc(1000 * sizeof(char));
    buffer_2[i] = malloc(1000 * sizeof(char));
    buffer_3[i] = malloc(1000 * sizeof(char));

    strcpy(buffer_1[i], "");
    strcpy(buffer_2[i], "");
    strcpy(buffer_3[i], "");
  }


  // cite: 6_5_prod_cons_pipeline.c
  // Create the threads
  pthread_create(&output_t, NULL, get_output_t, NULL);
  pthread_create(&plus_sign_t, NULL, get_plus_sign_t, NULL);
  pthread_create(&line_separator_t, NULL, get_line_separator_t, NULL);
  pthread_create(&input_t, NULL, get_input, NULL);
  
  
  
  // // Wait for the threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(line_separator_t, NULL);
  pthread_join(plus_sign_t, NULL);
  pthread_join(output_t, NULL);
  return EXIT_SUCCESS;
}
