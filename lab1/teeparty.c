#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define BUFFER_SIZE 32

typedef struct {
  int consumers;
  size_t buffer_size;
  char* buffer;
  pthread_cond_t* read_lock;
  pthread_mutex_t* write_lock;
  bool eof;
} buffer_t;

typedef struct {
  int id;
  FILE* fd;
  buffer_t* buffer;
} thread_spec;

/*
 * writes the contents of the buffer filled by reader to an open FILE*
 */
void* writer (void* arg) {
  thread_spec* specs = (thread_spec*) arg;
  int id = specs->id;
  buffer_t* buffer = specs->buffer;

  while (!buffer->eof) {
    pthread_mutex_lock(&buffer->write_lock[id]);
    // Wait for buffer to be filled
    pthread_cond_wait(&buffer->read_lock[id], &buffer->write_lock[id]);
    if (buffer->eof) {
      break; // on reaching end of file, stop reading buffer
    }
    // send the buffer to the file
    fprintf(specs->fd, "%s", buffer->buffer);
    // Tell reader to keep reading
    pthread_cond_signal(&buffer->read_lock[id]);
    pthread_mutex_unlock(&buffer->write_lock[id]);
  }

  return NULL;
}

/*
 * opens a reader which reads an incoming file descriptor and stores the values
 * in buffer
 */
void* reader (void* arg) {
  thread_spec* specs = (thread_spec*) arg;
  buffer_t* buffer = specs->buffer;
  size_t buffer_size = buffer->buffer_size;

  // Initialize the buffer
  char* tmpbuffer = malloc(buffer_size+1);
  char* tmpvar;
  char* read = "initial value";
  memset(tmpbuffer, 0, buffer_size+1);

  while(read != NULL) {
    read = fgets(tmpbuffer, buffer_size, specs->fd);
    tmpvar = buffer->buffer;

    // Obtain mutex lock for all writers
    for (int i = 0; i < buffer->consumers; i++) {
      pthread_mutex_lock(&buffer->write_lock[i]);
    }

    // Update the buffer pointer to a new position
    buffer->buffer = tmpbuffer;

    // Unlock all writers
    for (int i = 0; i < buffer->consumers; i++) {
      pthread_cond_signal(&buffer->read_lock[i]);
    }

    // Wait for writers and then drop the mutex
    for (int i = 0; i < buffer->consumers; i++) {
      pthread_cond_wait(&buffer->read_lock[i], &buffer->write_lock[i]);
      pthread_mutex_unlock(&buffer->write_lock[i]);
    }
    tmpbuffer = tmpvar; // Use the old buffer next time
    memset(tmpbuffer, 0, buffer_size+1);
  }

  // Obtain mutex lock for all writers
  for (int i = 0; i < buffer->consumers; i++) {
    pthread_mutex_lock(&buffer->write_lock[i]);
  }
  buffer->eof = true;
  // Make writers continue and then die
  for (int i = 0; i < buffer->consumers; i++) {
    pthread_cond_signal(&buffer->read_lock[i]);
    pthread_mutex_unlock(&buffer->write_lock[i]);
  }
  return NULL;
}

pthread_attr_t attr;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(-1);
  }

  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  // initialize buffer
  buffer_t buffer = {
    .consumers = 2,
    .buffer_size = BUFFER_SIZE,
    .eof = false
  };

  buffer.read_lock = malloc(sizeof(pthread_cond_t)*buffer.consumers);
  buffer.write_lock = malloc(sizeof(pthread_mutex_t)*buffer.consumers);
  buffer.buffer = malloc(buffer.buffer_size+1);
  memset(buffer.buffer, 0, buffer.buffer_size+1);
  for (int i = 0; i < buffer.consumers; i++) {
    pthread_mutex_init(&buffer.write_lock[i], NULL);
    pthread_cond_init(&buffer.read_lock[i], NULL);
  }

  FILE* out_file = fopen(argv[1], "w");

  // Set up the specifications for the writers
  thread_spec filewriter = {.id = 1,
                            .fd = out_file,
                            .buffer = &buffer};
  thread_spec stdoutwriter = {.id = 0,
                              .fd = stdout,
                              .buffer = &buffer};
  // Set up the specification for the stdin reader
  thread_spec stdinreader = {.fd = stdin,
                             .buffer = &buffer};

  pthread_t in, out, file;
  pthread_create(&file, &attr, writer, (void *) &filewriter);
  pthread_create(&out, &attr, writer, (void *) &stdoutwriter);
  pthread_create(&in, &attr, reader, (void *) &stdinreader);

  pthread_join(file, NULL);
  pthread_join(out, NULL);
  pthread_join(in, NULL);
}
