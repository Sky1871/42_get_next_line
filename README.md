*This project has been created as part of the 42 curriculum by kseltenr*

# Description

This projects is about creating a function that, allows to read a line ending with a newline character  from a file descriptor, without knowing its size beforehand.

## Mandatory Part

### Buffer structure

At the heart of this project is the custom buffer struct defined in get_next_line.h:
```c
typedef struct buffer
{
    char    *b_buf; // The actual string data
    int     b_len;  // The current length of the string
    int     b_cap;  // The total allocated memory capacity
}   buffer;
```
At first I tried to implement it using a simple char * static variable and reallocating it on every read, but that was rather slow and inefficient, so I've decided to use this buffer structure instead. This implementation acts like a C++ std::vector. It tracks its capacity (`b_cap`) and its actual length (`b_len`). If new data fits within the existing capacity, it is simply appended without calling malloc. Memory is only reallocated when the buffer runs out of space.

### Execution Step-by-Step

When `get_next_line(int fd)` is called, the program follows a 5-step lifecycle.

#### 1. Initialization
The function relies on a `static buffer buf`. Because it is static, its state persists across multiple function calls.

- On the very first call, `buf.b_buf` is `NULL`.

- The code initializes it with a default capacity of 256 bytes, sets the length to 0, and allocates the initial memory.

#### 2. Reading Data (read_to_buf)
The program needs to read from the file descriptor until it either finds a newline character or reaches the EOF.

- `find_line` checks if a `\n` already exists in the static buffer. If it does, reading stops immediately.

- If no newline is found, `read(fd, temp, BUFFER_SIZE)` pulls a chunk of data from the file into a temporary buffer.

- This temporary buffer is then appended to the static buffer using `ft_strjoin`.

#### 3. Appending (`ft_strjoin` & `ft_allocjoin`)
This is where the memory optimization happens in `get_next_line_utils.c`:

- In-Place Append: `ft_strjoin` checks if the current length plus the new data length is less than the total capacity (`buf->b_len + len < buf->b_cap`). If it fits, it writes the new characters directly into the existing allocated space.

- Reallocation: If the data exceeds the capacity, it triggers `ft_allocjoin`. This function creates a newly sized buffer (increasing the capacity by the total required length + existing capacity), copies the old data over, appends the new data, updates the capacity/length trackers, and frees the old memory.

#### 4. Extracting the Line (`return_line`)
Once a newline is found (or EOF is reached), the code must extract the exact line to return to the user.

- `return_line` iterates through the static buffer until it hits a `\n` or `\0`.

- It allocates a fresh string `ret` of the exact size needed.

- It copies characters from the static buffer into `ret`, including the newline character, null-terminates it, and prepares it for the user.

5. Cleaning Up for the Next Call (`delete_line`)
Before returning the extracted line, the static buffer must be updated so the next call to `get_next_line` starts exactly where the previous one left off.

- `delete_line` calculates how much data was just extracted.

- If the entire buffer was consumed, it frees the buffer completely and returns `NULL`.

- Otherwise, it allocates a new, smaller buffer, copies whatever "leftover" data remains after the newline, and frees the old buffer. It updates `b_len` and `b_cap` accordingly.

### File Breakdown

- `get_next_line.h`: Contains the prototypes, the `BUFFER_SIZE` macro safeguard, and the custom `buffer` struct definition.

- `get_next_line.c`: Contains the core logic. Manages the static variable, drives the read loop, and coordinates the extraction and cleanup phases.

- `get_next_line_utils.c`: Contains all helper functions. Focuses heavily on string manipulation (`ft_strlen`) and dynamic memory management (`ft_strjoin`, `ft_allocjoin`, `delete_line`, `return_line`).

### Justification
By carrying capacity metrics alongside the static buffer, this implementation drastically reduces the overhead of `malloc` and `free` operations, especially when reading files with very long lines or when compiled with a very small `BUFFER_SIZE`.

## Bonus Part

### Linked List Buffer Structure
To handle multiple file descriptors without losing the reading thread of any of them, the buffer struct is expanded into a Linked List node:
```c
typedef struct buffer
{
    char            *b_buf; // The actual string data
    int             b_len;  // The current length of the string
    int             b_cap;  // The total allocated memory capacity
    int             b_id;   // The File Descriptor (fd) associated with this buffer
    struct buffer   *next;  // Pointer to the next buffer in the linked list
}   buffer;
```
This structure retains the `b_len` and `b_cap` optimizations from the mandatory part (minimizing `malloc` calls) while adding `b_id` to identify which file is being read, and `next` to chain multiple files together.

### Execution Step-by-Step
When `get_next_line(int fd)` is called, the program executes the following lifecycle to ensure the correct file is processed:

#### 1. The Static Head Node
The function declares a single `static buffer buf;`. This static variable serves as the permanent head of the linked list across all function calls.

#### 2. Locating the Correct File (`find_node` & `create_node`)
Before reading any data, the program must find the buffer associated with the requested `fd`.

- Traversal: `find_node` iterates through the linked list starting from the static `buf`. It checks if `current->b_id == fd`.

- Creation: If it reaches the end of the list (`current->next == NULL`) without finding a match, it realizes this is the first time this `fd` is being read. It calls `create_node(fd)` to allocate a new node, initializes its capacity to 256 bytes, links it to the list, and assigns it the new `fd`.

- The function returns a pointer to the exact `current` buffer node belonging to the requested file descriptor.

#### 3. Reading and Vector-Style Appending (`read_to_buf`)
Once the correct node is isolated, the reading process behaves exactly like the mandatory version, but operates strictly on `current`:

- Data is read in chunks of `BUFFER_SIZE`.

- `ft_strjoin` checks if the new chunk fits within `current->b_cap`. If it does, it appends it directly. If not, `ft_allocjoin` expands the capacity.

#### 4. Extracting the Line (`return_line`)
The function extracts the line up to the first `\n` or `\0` from `current->b_buf`. This isolation guarantees that reading from `fd 4` will never accidentally return data read from `fd 3`.

#### 5. Node Cleanup (`delete_line`)
After the line is extracted, `delete_line` trims the returned data from the beginning of `current->b_buf` and leaves the remaining text intact for the next time this specific `fd` is called. The node remains in the linked list for future reads.

### Justification

This linked list approach is much better for memory efficiency.
If a program only opens three files, this implementation only allocates three nodes. It does not waste a bunch of empty array slots, like a static array would. Combined with the capacity-tracking vector logic (`b_cap`), this implementation dynamically scales both horizontally (adding new fds) and vertically (handling long lines) with minimal memory overhead and CPU cycles.

# Instructions

## 1. Compilation
Compile your files alongside the source files, defining the BUFFER_SIZE macro (optionally) using the -D flag:
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl
```
or
```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c -o gnl
```

## 2. Example Usage
```c
#include "get_next_line_bonus.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int     fd;
    char    *line;

    fd = open("file.txt", O_RDONLY);

    while ((line = get_next_line(fd)) != NULL)
    {
        printf("fd: %s", line);
        free(line);
    }

    close(fd);
    return (0);
}
```
or
```c
#include "get_next_line_bonus.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int     fd1;
    int     fd2;
    char    *line;

    fd1 = open("file1.txt", O_RDONLY);
    fd2 = open("file2.txt", O_RDONLY);

    while ((line = get_next_line(fd1)) != NULL)
    {
        printf("FD1: %s", line);
        free(line);
        
        line = get_next_line(fd2);
        if (line)
        {
            printf("FD2: %s", line);
            free(line);
        }
    }

    close(fd1);
    close(fd2);
    return (0);
}
```

# Resources
