/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    uint8_t offset = buffer->out_offs;

    int i = 0;

    for(i = 0; i < ENTRY_SIZE; i++) {
        if (buffer->entry[offset].size > char_offset) {
            // Found buffer entry has correct char_offset
            *entry_offset_byte_rtn = char_offset;
            return &buffer->entry[offset];
        } else {
            char_offset = char_offset - buffer->entry[offset].size;
            offset = (offset + 1) % ENTRY_SIZE;
        }
    }
    // not enough data is written. return NULL
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
const struct aesd_buffer_entry* aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    // This pointer stores point to existing entry at out_offs
    struct aesd_buffer_entry* out_offs = NULL;

    if (buffer->full == true) {
       out_offs = &buffer->entry[buffer->out_offs]; 
    }
    // Insert new entry to the buffer
    buffer->entry[buffer->in_offs] = *add_entry;

    // Increase buffer offset
    buffer->in_offs = (buffer->in_offs + 1) % ENTRY_SIZE;

    // When buffer is already full, move out_offs to the in_offs
    if (buffer->full == true) {
        buffer->out_offs = buffer->in_offs;
    }
    // mark buffer full
    if (buffer->in_offs == buffer->out_offs) {
        buffer->full = true;
    }
    else {
         buffer->full = false;
    }
    // This will be NULL or point to existing entry at out_offs
    return out_offs;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
