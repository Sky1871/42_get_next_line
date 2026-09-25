*This project has been created as part of the 42 curriculum by kseltenr*

# Description

This projects is about creating a function that, allows to read a line ending with a newline character  from a file descriptor, without knowing its size beforehand.

## Mandatory part

### Buffer structure

At the heart of this project is the custom buffer struct defined in get_next_line.h:
```
typedef struct buffer
{
    char    *b_buf; // The actual string data
    int     b_len;  // The current length of the string
    int     b_cap;  // The total allocated memory capacity
}   buffer;
```
At first I tried to implement it using a simple char * static variable and reallocating it on every read, but that was rather slow and inefficient, so I've decided to use this buffer structure instead. This implementation acts like a C++ std::vector. It tracks its capacity (`b_cap`) and its actual length (`b_len`). If new data fits within the existing capacity, it is simply appended without calling malloc. Memory is only reallocated when the buffer runs out of space.

### Execution step-by-step

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

# Instructions

# Resources
